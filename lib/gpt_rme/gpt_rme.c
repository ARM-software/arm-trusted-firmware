/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <common/debug.h>
#include <lib/extensions/rme.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#if !ENABLE_FEAT_RME
#error "ENABLE_FEAT_RME must be enabled to use the GPT library"
#endif

static uint32_t g_pps_bits;
static uint32_t g_pgs_bits;
static uint32_t g_l1_cnt_2mb;
static uint32_t g_l1_index_mask;

/*
 * Internal structure to retrieve the values from get_gpi_params();
 */
typedef struct {
	uint64_t gpt_l1_desc;
	uint64_t *gpt_l1_addr;
	unsigned int idx;
	unsigned int gpi_shift;
	unsigned int gpi;
#if (RME_GPT_BITLOCK_BLOCK != 0)
	bitlock_t *lock;
	LOCK_TYPE mask;
#endif
} gpi_info_t;

typedef void (*gpt_shatter_func)(uintptr_t base, const gpi_info_t *gpi_info,
					uint64_t l1_desc);
typedef void (*gpt_tlbi_func)(uintptr_t base);

/*
 * Look-up structure for
 * invalidating TLBs of GPT entries by Physical address, last level.
 */
typedef struct {
	gpt_tlbi_func function;
	size_t mask;
} gpt_tlbi_lookup_t;

static void shatter_2mb(uintptr_t base, const gpi_info_t *gpi_info,
			uint64_t l1_desc);
static void shatter_32mb(uintptr_t base, const gpi_info_t *gpi_info,
			 uint64_t l1_desc);
static void shatter_512mb(uintptr_t base, const gpi_info_t *gpi_info,
			  uint64_t l1_desc);

/* This value comes from read_gptbr_el3 */
#define GPT_L0BASE		((uintptr_t)((read_gptbr_el3() >> GPTBR_BADDR_SHIFT) & GPTBR_BADDR_MASK) << GPTBR_BADDR_VAL_SHIFT)

/* Number of 128-bit L1 entries in 2MB, 32MB and 512MB */
#define L1_QWORDS_2MB	(g_l1_cnt_2mb / 2U)
#define L1_QWORDS_32MB	(L1_QWORDS_2MB * 16U)
#define L1_QWORDS_512MB	(L1_QWORDS_32MB * 16U)

/* Size in bytes of L1 entries in 2MB, 32MB */
#define L1_BYTES_2MB	(g_l1_cnt_2mb * sizeof(uint64_t))
#define L1_BYTES_32MB	(L1_BYTES_2MB * 16U)

/* Get the index into the L1 table from a physical address */
#define GPT_L1_INDEX(_pa)						\
	(((_pa) >> (unsigned int)GPT_L1_IDX_SHIFT(g_pgs_bits)) & g_l1_index_mask)

/* These variables are used during runtime */
#if (RME_GPT_BITLOCK_BLOCK == 0)
/*
 * The GPTs are protected by a global spinlock to ensure
 * that multiple CPUs do not attempt to change the descriptors at once.
 */
static spinlock_t gpt_lock;

/* Lock/unlock macros for GPT entries
 *
 * Access to GPT is controlled by a global lock to ensure
 * that no more than one CPU is allowed to make changes at any
 * given time.
 */
#define GPT_LOCK	spin_lock(&gpt_lock)
#define GPT_UNLOCK	spin_unlock(&gpt_lock)
#else

/* Base address of bitlocks array */
static bitlock_t *gpt_bitlock;

/*
 * Access to a block of memory is controlled by a bitlock.
 * Size of block = RME_GPT_BITLOCK_BLOCK * 512MB.
 */
#define GPT_LOCK	bit_lock(gpi_info.lock, gpi_info.mask)
#define GPT_UNLOCK	bit_unlock(gpi_info.lock, gpi_info.mask)
#endif /* RME_GPT_BITLOCK_BLOCK */

static void tlbi_page_dsbosh(uintptr_t base)
{
	uint8_t pgs = EXTRACT(GPCCR_PGS, read_gpccr_el3());
	/* Look-up table for invalidation TLBs for 4KB, 16KB and 64KB pages */
	static const gpt_tlbi_lookup_t tlbi_page_lookup[] = {
		{ tlbirpalos_4k, ~(SZ_4K - 1UL) },
		{ tlbirpalos_64k, ~(SZ_64K - 1UL) },
		{ tlbirpalos_16k, ~(SZ_16K - 1UL) }
	};

	tlbi_page_lookup[pgs].function(base & tlbi_page_lookup[pgs].mask);
	dsbosh();
}

/*
 * Helper function to fill out GPI entries in a single L1 table
 * with Granules or Contiguous descriptor.
 *
 * Parameters
 *   l1			Pointer to 2MB, 32MB or 512MB aligned L1 table entry to fill out
 *   l1_desc		GPT Granules or Contiguous descriptor set this range to
 *   cnt		Number of double 128-bit L1 entries to fill
 *
 */
static void fill_desc(uint64_t *l1, uint64_t l1_desc, unsigned int cnt)
{
	uint128_t *l1_quad = (uint128_t *)l1;
	uint128_t l1_quad_desc = (uint128_t)l1_desc | ((uint128_t)l1_desc << 64);

	VERBOSE("GPT: %s(%p 0x%"PRIx64" %u)\n", __func__, l1, l1_desc, cnt);

	for (unsigned int i = 0U; i < cnt; i++) {
		*l1_quad++ = l1_quad_desc;
	}
}

static void shatter_2mb(uintptr_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned long idx = GPT_L1_INDEX(ALIGN_2MB(base));

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n",
				__func__, base, l1_desc);

	/* Convert 2MB Contiguous block to Granules */
	fill_desc(&gpi_info->gpt_l1_addr[idx], l1_desc, L1_QWORDS_2MB);
}

static void shatter_32mb(uintptr_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned long idx = GPT_L1_INDEX(ALIGN_2MB(base));
	const uint64_t *l1_gran = &gpi_info->gpt_l1_addr[idx];
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 2MB);
	uint64_t *l1;

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n",
				__func__, base, l1_desc);

	/* Get index corresponding to 32MB aligned address */
	idx = GPT_L1_INDEX(ALIGN_32MB(base));
	l1 = &gpi_info->gpt_l1_addr[idx];

	/* 16 x 2MB blocks in 32MB */
	for (unsigned int i = 0U; i < 16U; i++) {
		/* Fill with Granules or Contiguous descriptors */
		fill_desc(l1, (l1 == l1_gran) ? l1_desc : l1_cont_desc,
							L1_QWORDS_2MB);
		l1 = (uint64_t *)((uintptr_t)l1 + L1_BYTES_2MB);
	}
}

static void shatter_512mb(uintptr_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned long idx = GPT_L1_INDEX(ALIGN_32MB(base));
	const uint64_t *l1_32mb = &gpi_info->gpt_l1_addr[idx];
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 32MB);
	uint64_t *l1;

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n",
				__func__, base, l1_desc);

	/* Get index corresponding to 512MB aligned address */
	idx = GPT_L1_INDEX(ALIGN_512MB(base));
	l1 = &gpi_info->gpt_l1_addr[idx];

	/* 16 x 32MB blocks in 512MB */
	for (unsigned int i = 0U; i < 16U; i++) {
		if (l1 == l1_32mb) {
			/* Shatter this 32MB block */
			shatter_32mb(base, gpi_info, l1_desc);
		} else {
			/* Fill 32MB with Contiguous descriptors */
			fill_desc(l1, l1_cont_desc, L1_QWORDS_32MB);
		}

		l1 = (uint64_t *)((uintptr_t)l1 + L1_BYTES_32MB);
	}
}

/*
 * A helper to write the value (target_pas << gpi_shift) to the index of
 * the gpt_l1_addr.
 */
static inline void write_gpt(uint64_t *gpt_l1_desc, uint64_t *gpt_l1_addr,
			     unsigned int gpi_shift, unsigned int idx,
			     unsigned int target_pas)
{
	*gpt_l1_desc &= ~(GPT_L1_GRAN_DESC_GPI_MASK << gpi_shift);
	*gpt_l1_desc |= ((uint64_t)target_pas << gpi_shift);
	gpt_l1_addr[idx] = *gpt_l1_desc;

	dsboshst();
}

/*
 * Helper to retrieve the gpt_l1_* information from the base address
 * returned in gpi_info.
 */
static int get_gpi_params(uint64_t base, gpi_info_t *gpi_info)
{
	uint64_t gpt_l0_desc, *gpt_l0_base;
	__unused unsigned int block_idx;

	gpt_l0_base = (uint64_t *)GPT_L0BASE;
	gpt_l0_desc = gpt_l0_base[GPT_L0_IDX(base)];
	if (GPT_L0_TYPE(gpt_l0_desc) != GPT_L0_TYPE_TBL_DESC) {
		VERBOSE("GPT: Granule is not covered by a table descriptor!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		return -EINVAL;
	}

	/* Get the table index and GPI shift from PA */
	gpi_info->gpt_l1_addr = GPT_L0_TBLD_ADDR(gpt_l0_desc);
	gpi_info->idx = (unsigned int)GPT_L1_INDEX(base);
	gpi_info->gpi_shift = GPT_L1_GPI_IDX(g_pgs_bits, base) << 2;

#if (RME_GPT_BITLOCK_BLOCK != 0)
	/* Block index */
	block_idx = (unsigned int)(base / (RME_GPT_BITLOCK_BLOCK * SZ_512M));

	/* Bitlock address and mask */
	gpi_info->lock = &gpt_bitlock[block_idx / LOCK_BITS];
	gpi_info->mask = 1U << (block_idx & (LOCK_BITS - 1U));
#endif
	return 0;
}

/*
 * Helper to retrieve the gpt_l1_desc and GPI information from gpi_info.
 * This function is called with bitlock or spinlock acquired.
 */
static void read_gpi(gpi_info_t *gpi_info)
{
	gpi_info->gpt_l1_desc = (gpi_info->gpt_l1_addr)[gpi_info->idx];

	if ((gpi_info->gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
				 GPT_L1_TYPE_CONT_DESC) {
		/* Read GPI from Contiguous descriptor */
		gpi_info->gpi = (unsigned int)GPT_L1_CONT_GPI(gpi_info->gpt_l1_desc);
	} else {
		/* Read GPI from Granules descriptor */
		gpi_info->gpi = (unsigned int)((gpi_info->gpt_l1_desc >> gpi_info->gpi_shift) &
						GPT_L1_GRAN_DESC_GPI_MASK);
	}
}

static void flush_page_to_popa(uintptr_t addr)
{
	size_t size = GPT_PGS_ACTUAL_SIZE(g_pgs_bits);

	if (is_feat_mte2_supported()) {
		flush_dcache_to_popa_range_mte2(addr, size);
	} else {
		flush_dcache_to_popa_range(addr, size);
	}
}

/*
 * Helper function to check if all L1 entries in 2MB block have
 * the same Granules descriptor value.
 *
 * Parameters
 *   base		Base address of the region to be checked
 *   gpi_info		Pointer to 'gpt_config_t' structure
 *   l1_desc		GPT Granules descriptor with all entries
 *			set to the same GPI.
 *
 * Return
 *   true if L1 all entries have the same descriptor value, false otherwise.
 */
__unused static bool check_fuse_2mb(uint64_t base, const gpi_info_t *gpi_info,
					uint64_t l1_desc)
{
	/* Last L1 entry index in 2MB block */
	unsigned int long idx = GPT_L1_INDEX(ALIGN_2MB(base)) +
						g_l1_cnt_2mb - 1UL;

	/* Number of L1 entries in 2MB block */
	unsigned int cnt = g_l1_cnt_2mb;

	/*
	 * Start check from the last L1 entry and continue until the first
	 * non-matching to the passed Granules descriptor value is found.
	 */
	while (cnt-- != 0U) {
		if (gpi_info->gpt_l1_addr[idx--] != l1_desc) {
			/* Non-matching L1 entry found */
			return false;
		}
	}

	return true;
}

__unused static void fuse_2mb(uint64_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	/* L1 entry index of the start of xb
	   2MB block */
	unsigned long idx_2 = GPT_L1_INDEX(ALIGN_2MB(base));

	/* 2MB Contiguous descriptor */
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 2MB);

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n", __func__, base, l1_desc);

	fill_desc(&gpi_info->gpt_l1_addr[idx_2], l1_cont_desc, L1_QWORDS_2MB);
}

/*
 * Helper function to check if all 1st L1 entries of 2MB blocks
 * in 32MB have the same 2MB Contiguous descriptor value.
 *
 * Parameters
 *   base		Base address of the region to be checked
 *   gpi_info		Pointer to 'gpt_config_t' structure
 *   l1_desc		GPT Granules descriptor.
 *
 * Return
 *   true if all L1 entries have the same descriptor value, false otherwise.
 */
__unused static bool check_fuse_32mb(uint64_t base, const gpi_info_t *gpi_info,
					uint64_t l1_desc)
{
	/* The 1st L1 entry index of the last 2MB block in 32MB */
	unsigned long idx = GPT_L1_INDEX(ALIGN_32MB(base)) +
					(15UL * g_l1_cnt_2mb);

	/* 2MB Contiguous descriptor */
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 2MB);

	/* Number of 2MB blocks in 32MB */
	unsigned int cnt = 16U;

	/* Set the first L1 entry to 2MB Contiguous descriptor */
	gpi_info->gpt_l1_addr[GPT_L1_INDEX(ALIGN_2MB(base))] = l1_cont_desc;

	/*
	 * Start check from the 1st L1 entry of the last 2MB block and
	 * continue until the first non-matching to 2MB Contiguous descriptor
	 * value is found.
	 */
	while (cnt-- != 0U) {
		if (gpi_info->gpt_l1_addr[idx] != l1_cont_desc) {
			/* Non-matching L1 entry found */
			return false;
		}
		idx -= g_l1_cnt_2mb;
	}

	return true;
}

__unused static void fuse_32mb(uint64_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	/* L1 entry index of the start of 32MB block */
	unsigned long idx_32 = GPT_L1_INDEX(ALIGN_32MB(base));

	/* 32MB Contiguous descriptor */
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 32MB);

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n", __func__, base, l1_desc);

	fill_desc(&gpi_info->gpt_l1_addr[idx_32], l1_cont_desc, L1_QWORDS_32MB);
}

/*
 * Helper function to check if all 1st L1 entries of 32MB blocks
 * in 512MB have the same 32MB Contiguous descriptor value.
 *
 * Parameters
 *   base		Base address of the region to be checked
 *   gpi_info		Pointer to 'gpt_config_t' structure
 *   l1_desc		GPT Granules descriptor.
 *
 * Return
 *   true if all L1 entries have the same descriptor value, false otherwise.
 */
__unused static bool check_fuse_512mb(uint64_t base, const gpi_info_t *gpi_info,
					uint64_t l1_desc)
{
	/* The 1st L1 entry index of the last 32MB block in 512MB */
	unsigned long idx = GPT_L1_INDEX(ALIGN_512MB(base)) +
					(15UL * 16UL * g_l1_cnt_2mb);

	/* 32MB Contiguous descriptor */
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 32MB);

	/* Number of 32MB blocks in 512MB */
	unsigned int cnt = 16U;

	/* Set the first L1 entry to 2MB Contiguous descriptor */
	gpi_info->gpt_l1_addr[GPT_L1_INDEX(ALIGN_32MB(base))] = l1_cont_desc;

	/*
	 * Start check from the 1st L1 entry of the last 32MB block and
	 * continue until the first non-matching to 32MB Contiguous descriptor
	 * value is found.
	 */
	while (cnt-- != 0U) {
		if (gpi_info->gpt_l1_addr[idx] != l1_cont_desc) {
			/* Non-matching L1 entry found */
			return false;
		}
		idx -= 16UL * g_l1_cnt_2mb;
	}

	return true;
}

__unused static void fuse_512mb(uint64_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	/* L1 entry index of the start of 512MB block */
	unsigned long idx_512 = GPT_L1_INDEX(ALIGN_512MB(base));

	/* 512MB Contiguous descriptor */
	uint64_t l1_cont_desc = GPT_L1_CONT_DESC(l1_desc, 512MB);

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n", __func__, base, l1_desc);

	fill_desc(&gpi_info->gpt_l1_addr[idx_512], l1_cont_desc, L1_QWORDS_512MB);
}

/*
 * Helper function to convert GPI entries in a single L1 table
 * from Granules to Contiguous descriptor.
 *
 * Parameters
 *   base		Base address of the region to be written
 *   gpi_info		Pointer to 'gpt_config_t' structure
 *   l1_desc		GPT Granules descriptor with all entries
 *			set to the same GPI.
 */
__unused static void fuse_block(uint64_t base, const gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	/* Start with check for 2MB block */
	if (!check_fuse_2mb(base, gpi_info, l1_desc)) {
		/* Check for 2MB fusing failed */
		return;
	}

#if (RME_GPT_MAX_BLOCK == 2)
	fuse_2mb(base, gpi_info, l1_desc);
#else
	/* Check for 32MB block */
	if (!check_fuse_32mb(base, gpi_info, l1_desc)) {
		/* Check for 32MB fusing failed, fuse to 2MB */
		fuse_2mb(base, gpi_info, l1_desc);
		return;
	}

#if (RME_GPT_MAX_BLOCK == 32)
	fuse_32mb(base, gpi_info, l1_desc);
#else
	/* Check for 512MB block */
	if (!check_fuse_512mb(base, gpi_info, l1_desc)) {
		/* Check for 512MB fusing failed, fuse to 32MB */
		fuse_32mb(base, gpi_info, l1_desc);
		return;
	}

	/* Fuse to 512MB */
	fuse_512mb(base, gpi_info, l1_desc);

#endif	/* RME_GPT_MAX_BLOCK == 32 */
#endif	/* RME_GPT_MAX_BLOCK == 2 */
}

/*
 * Helper function to convert GPI entries in a single L1 table
 * from Contiguous to Granules descriptor. This function updates
 * descriptor to Granules in passed 'gpt_config_t' structure as
 * the result of shuttering.
 *
 * Parameters
 *   base		Base address of the region to be written
 *   gpi_info		Pointer to 'gpt_config_t' structure
 *   l1_desc		GPT Granules descriptor set this range to.
 */
__unused static void shatter_block(uint64_t base, gpi_info_t *gpi_info,
				   uint64_t l1_desc)
{
	/* Look-up table for 2MB, 32MB and 512MB locks shattering */
	static const gpt_shatter_func gpt_shatter_lookup[] = {
		shatter_2mb,
		shatter_32mb,
		shatter_512mb
	};

	/* Look-up table for invalidation TLBs for 2MB, 32MB and 512MB blocks */
	static const gpt_tlbi_lookup_t tlbi_lookup[] = {
		{ tlbirpalos_2m, ~(SZ_2M - 1UL) },
		{ tlbirpalos_32m, ~(SZ_32M - 1UL) },
		{ tlbirpalos_512m, ~(SZ_512M - 1UL) }
	};

	/* Get shattering level from Contig field of Contiguous descriptor */
	unsigned long level = GPT_L1_CONT_CONTIG(gpi_info->gpt_l1_desc) - 1UL;

	/* Shatter contiguous block */
	gpt_shatter_lookup[level](base, gpi_info, l1_desc);

	tlbi_lookup[level].function(base & tlbi_lookup[level].mask);
	dsbosh();

	/*
	 * Update 'gpt_config_t' structure's descriptor to Granules to reflect
	 * the shattered GPI back to caller.
	 */
	gpi_info->gpt_l1_desc = l1_desc;
}

/*
 * This function is the granule transition delegate service. When a granule
 * transition request occurs it is routed to this function to have the request,
 * if valid, fulfilled following A1.1.1 Delegate of RME supplement.
 *
 * TODO: implement support for transitioning multiple granules at once.
 *
 * Parameters
 *   base		Base address of the region to transition, must be
 *			aligned to granule size.
 *   size		Size of region to transition, must be aligned to granule
 *			size.
 *   src_sec_state	Security state of the caller.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_delegate_pas(uint64_t base, size_t size, unsigned int src_sec_state)
{
	gpi_info_t gpi_info;
	uint64_t nse, __unused l1_desc;
	unsigned int target_pas;
	int res;

	/* Ensure that the tables have been set up before taking requests */
	assert(GPT_L0BASE != 0UL);

	/* Ensure that caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0UL);

	/* See if this is a single or a range of granule transition */
	if (size != GPT_PGS_ACTUAL_SIZE(g_pgs_bits)) {
		return -EINVAL;
	}

	/* Check that base and size are valid */
	if ((ULONG_MAX - base) < size) {
		VERBOSE("GPT: Transition request address overflow!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Make sure base and size are valid */
	if (((base & (GPT_PGS_ACTUAL_SIZE(g_pgs_bits) - 1UL)) != 0UL) ||
	    ((size & (GPT_PGS_ACTUAL_SIZE(g_pgs_bits) - 1UL)) != 0UL) ||
	    (size == 0UL) ||
	    ((base + size) >= GPT_PPS_ACTUAL_SIZE(g_pps_bits))) {
		VERBOSE("GPT: Invalid granule transition address range!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Delegate request can only come from REALM or SECURE */
	if ((src_sec_state != SMC_FROM_REALM) &&
	    (src_sec_state != SMC_FROM_SECURE)) {
		VERBOSE("GPT: Invalid caller security state 0x%x\n",
							src_sec_state);
		return -EINVAL;
	}

	if (src_sec_state == SMC_FROM_REALM) {
		target_pas = GPT_GPI_REALM;
		nse = (uint64_t)GPT_NSE_REALM << GPT_NSE_SHIFT;
		l1_desc = GPT_L1_REALM_DESC;
	} else {
		target_pas = GPT_GPI_SECURE;
		nse = (uint64_t)GPT_NSE_SECURE << GPT_NSE_SHIFT;
		l1_desc = GPT_L1_SECURE_DESC;
	}

	res = get_gpi_params(base, &gpi_info);
	if (res != 0) {
		return res;
	}

	/*
	 * Access to GPT is controlled by a lock to ensure that no more
	 * than one CPU is allowed to make changes at any given time.
	 */
	GPT_LOCK;
	read_gpi(&gpi_info);

	/* Check that the current address is in NS state */
	if (gpi_info.gpi != GPT_GPI_NS) {
		VERBOSE("GPT: Only Granule in NS state can be delegated.\n");
		VERBOSE("      Caller: %u, Current GPI: %u\n", src_sec_state,
			gpi_info.gpi);
		GPT_UNLOCK;
		return -EPERM;
	}

#if (RME_GPT_MAX_BLOCK != 0)
	/* Check for Contiguous descriptor */
	if ((gpi_info.gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
					GPT_L1_TYPE_CONT_DESC) {
		shatter_block(base, &gpi_info, GPT_L1_NS_DESC);
	}
#endif
	/*
	 * In order to maintain mutual distrust between Realm and Secure
	 * states, remove any data speculatively fetched into the target
	 * physical address space.
	 * Issue DC CIPAPA or DC_CIGDPAPA on implementations with FEAT_MTE2.
	 */
	flush_page_to_popa(base | nse);

	write_gpt(&gpi_info.gpt_l1_desc, gpi_info.gpt_l1_addr,
		  gpi_info.gpi_shift, gpi_info.idx, target_pas);

	/* Ensure that all agents observe the new configuration */
	tlbi_page_dsbosh(base);

	nse = (uint64_t)GPT_NSE_NS << GPT_NSE_SHIFT;

	/* Ensure that the scrubbed data have made it past the PoPA */
	flush_page_to_popa(base | nse);

#if (RME_GPT_MAX_BLOCK != 0)
	if (gpi_info.gpt_l1_desc == l1_desc) {
		/* Try to fuse */
		fuse_block(base, &gpi_info, l1_desc);
	}
#endif

	/* Unlock the lock to GPT */
	GPT_UNLOCK;

	/*
	 * The isb() will be done as part of context
	 * synchronization when returning to lower EL.
	 */
	VERBOSE("GPT: Granule 0x%"PRIx64" GPI 0x%x->0x%x\n",
		base, gpi_info.gpi, target_pas);

	return 0;
}

/*
 * This function is the granule transition undelegate service. When a granule
 * transition request occurs it is routed to this function where the request is
 * validated then fulfilled if possible.
 *
 * TODO: implement support for transitioning multiple granules at once.
 *
 * Parameters
 *   base		Base address of the region to transition, must be
 *			aligned to granule size.
 *   size		Size of region to transition, must be aligned to granule
 *			size.
 *   src_sec_state	Security state of the caller.
 *
 * Return
 *    Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_undelegate_pas(uint64_t base, size_t size, unsigned int src_sec_state)
{
	gpi_info_t gpi_info;
	uint64_t nse, __unused l1_desc;
	int res;

	/* Ensure that the tables have been set up before taking requests */
	assert(GPT_L0BASE != 0UL);

	/* Ensure that MMU and caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0UL);

	/* See if this is a single or a range of granule transition */
	if (size != GPT_PGS_ACTUAL_SIZE(g_pgs_bits)) {
		return -EINVAL;
	}

	/* Check that base and size are valid */
	if ((ULONG_MAX - base) < size) {
		VERBOSE("GPT: Transition request address overflow!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Make sure base and size are valid */
	if (((base & (GPT_PGS_ACTUAL_SIZE(g_pgs_bits) - 1UL)) != 0UL) ||
	    ((size & (GPT_PGS_ACTUAL_SIZE(g_pgs_bits) - 1UL)) != 0UL) ||
	    (size == 0UL) ||
	    ((base + size) >= GPT_PPS_ACTUAL_SIZE(g_pps_bits))) {
		VERBOSE("GPT: Invalid granule transition address range!\n");
		VERBOSE("      Base=0x%"PRIx64"\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	res = get_gpi_params(base, &gpi_info);
	if (res != 0) {
		return res;
	}

	/*
	 * Access to GPT is controlled by a lock to ensure that no more
	 * than one CPU is allowed to make changes at any given time.
	 */
	GPT_LOCK;
	read_gpi(&gpi_info);

	/* Check that the current address is in the delegated state */
	if ((src_sec_state == SMC_FROM_REALM) &&
		(gpi_info.gpi == GPT_GPI_REALM)) {
		l1_desc = GPT_L1_REALM_DESC;
		nse = (uint64_t)GPT_NSE_REALM << GPT_NSE_SHIFT;
	} else if ((src_sec_state == SMC_FROM_SECURE) &&
		(gpi_info.gpi == GPT_GPI_SECURE)) {
		l1_desc = GPT_L1_SECURE_DESC;
		nse = (uint64_t)GPT_NSE_SECURE << GPT_NSE_SHIFT;
	} else {
		VERBOSE("GPT: Only Granule in REALM or SECURE state can be undelegated\n");
		VERBOSE("      Caller: %u Current GPI: %u\n", src_sec_state,
			gpi_info.gpi);
		GPT_UNLOCK;
		return -EPERM;
	}

#if (RME_GPT_MAX_BLOCK != 0)
	/* Check for Contiguous descriptor */
	if ((gpi_info.gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
					GPT_L1_TYPE_CONT_DESC) {
		shatter_block(base, &gpi_info, l1_desc);
	}
#endif
	/*
	 * In order to maintain mutual distrust between Realm and Secure
	 * states, remove access now, in order to guarantee that writes
	 * to the currently-accessible physical address space will not
	 * later become observable.
	 */
	write_gpt(&gpi_info.gpt_l1_desc, gpi_info.gpt_l1_addr,
		  gpi_info.gpi_shift, gpi_info.idx, GPT_GPI_NO_ACCESS);

	/* Ensure that all agents observe the new NO_ACCESS configuration */
	tlbi_page_dsbosh(base);

	/* Ensure that the scrubbed data have made it past the PoPA */
	flush_page_to_popa(base | nse);

	/*
	 * Remove any data loaded speculatively in NS space from before
	 * the scrubbing.
	 */
	nse = (uint64_t)GPT_NSE_NS << GPT_NSE_SHIFT;

	flush_page_to_popa(base | nse);

	/* Clear existing GPI encoding and transition granule */
	write_gpt(&gpi_info.gpt_l1_desc, gpi_info.gpt_l1_addr,
		  gpi_info.gpi_shift, gpi_info.idx, GPT_GPI_NS);

	/* Ensure that all agents observe the new NS configuration */
	tlbi_page_dsbosh(base);

#if (RME_GPT_MAX_BLOCK != 0)
	if (gpi_info.gpt_l1_desc == GPT_L1_NS_DESC) {
		/* Try to fuse */
		fuse_block(base, &gpi_info, GPT_L1_NS_DESC);
	}
#endif
	/* Unlock the lock to GPT */
	GPT_UNLOCK;

	/*
	 * The isb() will be done as part of context
	 * synchronization when returning to lower EL.
	 */
	VERBOSE("GPT: Granule 0x%"PRIx64" GPI 0x%x->0x%x\n",
		base, gpi_info.gpi, GPT_GPI_NS);

	return 0;
}

/*
 * Public API to initialize the runtime variables required for granule transition
 * using delegate/undelegate calls. This initializes GPT bitlocks.
 *
 * Granule protection checks must be enabled already or this function will
 * return an error.
 *
 * Parameters
 *   l1_bitlocks_base	Base address of memory for L1 tables bitlocks.
 *   l1_bitlocks_size	Total size of memory available for L1 tables bitlocks.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_runtime_init(uintptr_t l1_bitlocks_base, size_t l1_bitlocks_size)
{
	uint8_t pgs;
	uint8_t pps;
	uint8_t pps_to_bits[] = { 32, 36, 40, 42, 44, 48, 52, 56 };
	uint8_t pgs_to_bits[] = { 12, 16, 14 };
	__unused size_t locks_size;

	/* Ensure that MMU and Data caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0UL);

	/* Ensure GPC are already enabled */
	if ((read_gpccr_el3() & GPCCR_GPC_BIT) == 0UL) {
		ERROR("GPT: Granule protection checks are not enabled!\n");
		return -EPERM;
	}

	pps = EXTRACT(GPCCR_PPS, read_gpccr_el3());
	pgs = EXTRACT(GPCCR_PGS, read_gpccr_el3());

	/* Read GPCCR to get PGS and PPS values */
	g_pps_bits = pps_to_bits[pps];
	g_pgs_bits = pgs_to_bits[pgs];
	g_l1_cnt_2mb = GPT_L1_ENTRY_COUNT_2MB(g_pgs_bits);
	g_l1_index_mask = GPT_L1_IDX_MASK(g_pgs_bits);

#if (RME_GPT_BITLOCK_BLOCK != 0)
	/*
	 * Size of GPT bitlocks in bytes for the protected address space
	 * with RME_GPT_BITLOCK_BLOCK * 512MB per bitlock.
	 */
	locks_size = GPT_PPS_ACTUAL_SIZE(g_pps_bits) /
			(RME_GPT_BITLOCK_BLOCK * SZ_512M * 8U);
	/*
	 * If protected space size is less than the size covered
	 * by 'bitlock' structure, check for a single bitlock.
	 */
	if (locks_size < LOCK_SIZE) {
		locks_size = LOCK_SIZE;
	/* Check bitlocks array size */
	} else if (locks_size > l1_bitlocks_size) {
		ERROR("GPT: Inadequate GPT bitlocks memory\n");
		ERROR("      Expected 0x%lx bytes, got 0x%lx\n",
			locks_size, l1_bitlocks_size);
		return -ENOMEM;
	}

	gpt_bitlock = (bitlock_t *)l1_bitlocks_base;

	/* Initialise GPT bitlocks */
	(void)memset((void *)gpt_bitlock, 0, locks_size);

	/* Flush GPT bitlocks to memory */
	flush_dcache_range((uintptr_t)gpt_bitlock, locks_size);
	INFO("*  Bitlocks:  0x%"PRIxPTR"/0x%lx\n", (uintptr_t)gpt_bitlock,
					locks_size);
#endif /* RME_GPT_BITLOCK_BLOCK */

	return 0;
}

#if FIRME_SUPPORT
void firme_gpt_lock(uint64_t base)
{
#if (RME_GPT_BITLOCK_BLOCK == 0)
	spin_lock(&gpt_lock);
#else
	unsigned int block_idx;
	bitlock_t *lock;
	LOCK_TYPE mask;

	block_idx = (unsigned int)(base / (RME_GPT_BITLOCK_BLOCK * SZ_512M));

	lock = (bitlock_t *)&gpt_bitlock[block_idx / LOCK_BITS];
	mask = 1U << (block_idx & (LOCK_BITS - 1U));

	bit_lock(lock, mask);
#endif
}

void firme_gpt_unlock(uint64_t base)
{
#if (RME_GPT_BITLOCK_BLOCK == 0)
	spin_unlock(&gpt_lock);
#else
	unsigned int block_idx;
	bitlock_t *lock;
	LOCK_TYPE mask;

	block_idx = (unsigned int)(base / (RME_GPT_BITLOCK_BLOCK * SZ_512M));

	lock = (bitlock_t *)&gpt_bitlock[block_idx / LOCK_BITS];
	mask = 1U << (block_idx & (LOCK_BITS - 1U));

	bit_unlock(lock, mask);
#endif
}
#endif /* FIRME_SUPPORT */
