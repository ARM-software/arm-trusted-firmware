/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>

#include "firme_gm_private.h"

#include <arch.h>
#include <arch_features.h>
#include <common/debug.h>
#include <lib/extensions/rme.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/firme/firme_granule_mgmt.h>
#include <services/firme_svc.h>

/* This value comes from read_gptbr_el3 */
#define GPT_L0BASE	((uintptr_t)((read_gptbr_el3() >> GPTBR_BADDR_SHIFT) \
			& GPTBR_BADDR_MASK) << GPTBR_BADDR_VAL_SHIFT)

/* Get the descriptor or NSE bits from GPI encoding. */
#define GPI_TO_DESC(_gpi)	(firme_gpi_config[_gpi].desc)
#define GPI_TO_NSE(_gpi)	\
	(((uint64_t)firme_gpi_config[_gpi].nse << GPT_NSE_SHIFT) | \
	 ((uint64_t)firme_gpi_config[_gpi].nse2 << GPT_NSE2_SHIFT))

/* Get the index into the L1 table from a physical address */
#define GPT_L1_INDEX(_pa)	\
	(((_pa) >> (unsigned int)GPT_L1_IDX_SHIFT(firme_gpt_config.pgs_bits)) & \
	 GPT_L1_IDX_MASK(firme_gpt_config.pgs_bits))

/* Number of 128-bit L1 entries in 2MB, 32MB and 512MB */
#define L1_QWORDS_2MB	(firme_gpt_config.l1_cnt_2mb / 2U)
#define L1_QWORDS_32MB	(L1_QWORDS_2MB * 16U)
#define L1_QWORDS_512MB	(L1_QWORDS_32MB * 16U)

/* Size in bytes of L1 entries in 2MB, 32MB */
#define L1_BYTES_2MB	(firme_gpt_config.l1_cnt_2mb * sizeof(uint64_t))
#define L1_BYTES_32MB	(L1_BYTES_2MB * 16U)

/*
 * Lookup table used to speed up granule transitions by associating GPIs and
 * relevant information such as descriptors, NSE fields, and transition
 * policies defined by FIRME rules.
 */
typedef struct {
	uint64_t desc;
	uint8_t nse;
	uint8_t nse2;
	uint16_t policy[3];
} firme_gpi_config_t;

/*
 * Internal structure to retrieve the values from get_gpi_params();
 * todo: extend this to gpi_geometry
 */
typedef struct {
	uint64_t gpt_l1_desc;
	uint64_t *gpt_l1_addr;
	unsigned int idx;
	unsigned int gpi_shift;
	unsigned int gpi;
} firme_gpi_info_t;

typedef struct {
	uint8_t pps_bits;
	uint8_t pgs_bits;

	/* Number of L1 entries in 2MB, depending on GPCCR_EL3.PGS */
	uint8_t l1_cnt_2mb;
} firme_gpt_config_t;

typedef void (*gpt_shatter_func)(uintptr_t base,
				 const firme_gpi_info_t *gpi_info,
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

static firme_gpt_config_t firme_gpt_config;

/*
 * This table allows us to easily look up GPI-specific information such as
 * descriptors, nse/nse2 fields, and allowed transitions without using large
 * blocks of nested conditionals which both take up a lot of space and are
 * very slow.
 *
 * This array contains an entry for each valid GPI, unused values are all zeros.
 * GPIs such as SA, NSP, and NSO are not part of base FEAT_RME so those policy
 * flags are set at runtime when FEAT_RME_GDI and FEAT_RME_GPC2 are enabled.
 *
 * uint64_t desc       The L1 descriptor associated with a GPI
 * uint8_t nse         NSE bits
 * uint8_t nse2        NSE2 bits
 * uint16_t policy[3] Contains bit fields representing which GPIs a given
 *                     security state can transition this GPI to. 0=s, 1=ns, and
 *                     0x2=realm.
 */
static firme_gpi_config_t firme_gpi_config[] = {
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ GPT_L1_SA_DESC, 0, GPT_NSE2_SA, { 0x0, 0x0, 0x0 } },
	{ GPT_L1_NSP_DESC, 0, GPT_NSE2_NSP, { 0x0, 0x0, 0x0 } },
	{ 0 },
	{ 0 },
	{ GPT_L1_SECURE_DESC,
	  GPT_NSE_SECURE,
	  0,
	  { (1 << GPT_GPI_NS), 0x0, 0x0 } },
	{ GPT_L1_NS_DESC,
	  GPT_NSE_NS,
	  0,
	  { (1 << GPT_GPI_SECURE), 0x0, (1 << GPT_GPI_REALM) } },
	{ GPT_L1_ROOT_DESC, GPT_NSE_ROOT, 0, { 0x0, 0x0, 0x0 } },
	{ GPT_L1_REALM_DESC, GPT_NSE_REALM, 0, { 0x0, 0x0, (1 << GPT_GPI_NS) } },
	{ 0 },
	{ GPT_L1_NSO_DESC, GPT_NSE_NS, 0, { 0x0, 0x0, 0x0 } },
	{ 0 },
	{ 0 },
};

static void fill_desc(uint64_t *l1, uint64_t l1_desc, unsigned int cnt)
{
	uint128_t *l1_quad = (uint128_t *)l1;
	uint128_t l1_quad_desc = (uint128_t)l1_desc | ((uint128_t)l1_desc << 64);

	VERBOSE("GPT: %s(%p 0x%"PRIx64" %u)\n", __func__, l1, l1_desc, cnt);

	for (unsigned int i = 0U; i < cnt; i++) {
		*l1_quad++ = l1_quad_desc;
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
__unused static bool check_fuse_2mb(uint64_t base,
				    const firme_gpi_info_t *gpi_info,
				    uint64_t l1_desc)
{
	/* Last L1 entry index in 2MB block */
	unsigned int long idx = GPT_L1_INDEX(ALIGN_2MB(base)) +
						firme_gpt_config.l1_cnt_2mb - 1UL;

	/* Number of L1 entries in 2MB block */
	unsigned int cnt = firme_gpt_config.l1_cnt_2mb;

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

__unused static void fuse_2mb(uint64_t base, const firme_gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	/* L1 entry index of the start of 2MB block */
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
__unused static bool check_fuse_32mb(uint64_t base, const firme_gpi_info_t *gpi_info,
					uint64_t l1_desc)
{
	/* The 1st L1 entry index of the last 2MB block in 32MB */
	unsigned long idx = GPT_L1_INDEX(ALIGN_32MB(base)) +
					(15UL * firme_gpt_config.l1_cnt_2mb);

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
		idx -= firme_gpt_config.l1_cnt_2mb;
	}

	return true;
}

__unused static void fuse_32mb(uint64_t base, const firme_gpi_info_t *gpi_info,
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
__unused static bool check_fuse_512mb(uint64_t base, const firme_gpi_info_t *gpi_info,
					uint64_t l1_desc)
{
	/* The 1st L1 entry index of the last 32MB block in 512MB */
	unsigned long idx = GPT_L1_INDEX(ALIGN_512MB(base)) +
					(15UL * 16UL * firme_gpt_config.l1_cnt_2mb);

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
		idx -= 16UL * firme_gpt_config.l1_cnt_2mb;
	}

	return true;
}

__unused static void fuse_512mb(uint64_t base, const firme_gpi_info_t *gpi_info,
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
__unused static void fuse_block(uint64_t base, const firme_gpi_info_t *gpi_info,
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

static void shatter_2mb(uintptr_t base, const firme_gpi_info_t *gpi_info,
				uint64_t l1_desc)
{
	unsigned long idx = GPT_L1_INDEX(ALIGN_2MB(base));

	VERBOSE("GPT: %s(0x%"PRIxPTR" 0x%"PRIx64")\n",
				__func__, base, l1_desc);

	/* Convert 2MB Contiguous block to Granules */
	fill_desc(&gpi_info->gpt_l1_addr[idx], l1_desc, L1_QWORDS_2MB);
}

static void shatter_32mb(uintptr_t base, const firme_gpi_info_t *gpi_info,
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

static void shatter_512mb(uintptr_t base, const firme_gpi_info_t *gpi_info,
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
__unused static void shatter_block(uint64_t base, firme_gpi_info_t *gpi_info,
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

static void read_gpi(firme_gpi_info_t *gpi_info)
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

/*
 * This function checks to see if a GPI value is valid.
 * todo: is_gpi_transition_permitted check should be enough?
 *
 * Parameters
 *   gpi		GPI to check for validity.
 *
 * Return
 *   true for a valid GPI, false for an invalid one.
 */
static bool is_gpi_valid(unsigned int gpi)
{
	switch (gpi) {
	case GPT_GPI_NO_ACCESS:
	case GPT_GPI_SECURE:
	case GPT_GPI_NS:
	case GPT_GPI_ROOT:
#if ENABLE_RMM
	case GPT_GPI_REALM:
#endif
	case GPT_GPI_ANY:
		return true;
	case GPT_GPI_NSO:
		return is_feat_rme_gpc2_supported();
	case GPT_GPI_SA:
	case GPT_GPI_NSP:
		return is_feat_rme_gdi_supported();
	default:
		return false;
	}
}

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

static inline bool is_gpi_transition_permitted(uint8_t caller,
					       uint8_t current_gpi,
					       uint8_t target_gpi)
{
	/*
	 * So we can use a small lookup table, change caller security state 0x21
	 * (from realm) to 0x2 so it can be an index.
	 */
	if (caller == SMC_FROM_REALM) {
		caller = 0x2;
	}

	assert(caller <= 0x2);
	assert(current_gpi <= GPT_GPI_ANY);

	return (firme_gpi_config[current_gpi].policy[caller] >> target_gpi) & 0x1;
}

static inline void gpt_write_entry(uint64_t base, uint8_t target_gpi,
				   firme_gpi_info_t *gpi_info)
{
	/* Update the GPI entry to the new state. */
	write_gpt(&gpi_info->gpt_l1_desc, gpi_info->gpt_l1_addr,
		  gpi_info->gpi_shift, gpi_info->idx, target_gpi);

	/* Ensure all agents observe new state. */
	tlbi_page_dsbosh(base);
}

static void flush_page_to_popa(uintptr_t addr)
{
	size_t size = GPT_PGS_ACTUAL_SIZE(firme_gpt_config.pgs_bits);

	if (is_feat_mte2_supported()) {
		flush_dcache_to_popa_range_mte2(addr, size);
	} else {
		flush_dcache_to_popa_range(addr, size);
	}
}

static inline void gpt_delegate(uint64_t base, uint8_t target_gpi,
				firme_gpi_info_t *gpi_info)
{
	uint8_t source_gpi = gpi_info->gpi;

	/*
	 * In order to maintain mutual distrust between states, remove any data
	 * speculatively fetched into the target physical address space.
	 */
	flush_page_to_popa(base | GPI_TO_NSE(target_gpi));

	gpt_write_entry(base, target_gpi, gpi_info);

	/* Ensure scrubbed data has made it past PoPA */
	flush_page_to_popa(base | GPI_TO_NSE(source_gpi));
}

static inline void gpt_undelegate(uint64_t base, uint8_t target_gpi,
				  firme_gpi_info_t *gpi_info)
{
	uint8_t source_gpi = gpi_info->gpi;

	/*
	 * In order to maintain mutual distrust between states, remove access
	 * now, in order to guarantee that writes to the currently-accessible
	 * physical address space will not later become observable.
	 */
	write_gpt(&gpi_info->gpt_l1_desc, gpi_info->gpt_l1_addr,
		  gpi_info->gpi_shift, gpi_info->idx, GPT_GPI_NO_ACCESS);

	/* Ensure all agents observe NO ACCESS state. */
	tlbi_page_dsbosh(base);

	/*
	 * Ensure that the scrubbed data have made it past the PoPA for both
	 * old and new security states.
	 */
	flush_page_to_popa(base | GPI_TO_NSE(source_gpi));
	flush_page_to_popa(base | GPI_TO_NSE(target_gpi));

	gpt_write_entry(base, target_gpi, gpi_info);
}

/*
 * Helper to retrieve the gpt_l1_* information from the base address
 * returned in gpi_info.
 */
static int get_gpi_params(uint64_t base, firme_gpi_info_t *gpi_info)
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
	gpi_info->gpi_shift = GPT_L1_GPI_IDX(firme_gpt_config.pgs_bits, base) << 2;

	return 0;
}

/*
 * This function is for firme handler to set GPI encoding to 'target_gpi'
 * based on current_gpi and src_sec_state.
 *
 * The transition of GPI from source to target are based on FIRME rules
 *
 * Parameters
 *   base               Base address of the first granule to transition, aligned
 *                      to granule size.
 *   target_gpi         GPI to transition the granules to.
 *   src_sec_state      Security state of the requesting entity. This will be
 *                      combined with target_gpi to determine whether a
 *                      transition is allowed.
 */
static int gm_gpi_set(uint64_t base, uint8_t target_gpi, uint8_t src_sec_state)
{
	firme_gpi_info_t gpi_info = { 0, NULL, 0, 0, 0 };
	int res;
	size_t size;

	/* Ensure that the tables have been set up before taking requests */
	assert((unsigned long)GPT_L0BASE != 0UL);

	/* Ensure that MMU and caches are enabled */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0UL);

	/* Calculate total region size and zero out granule count. */
	size = GPT_PGS_ACTUAL_SIZE(firme_gpt_config.pgs_bits);

	/* Make sure target GPI is valid. */
	if (!is_gpi_valid(target_gpi)) {
		VERBOSE("GPT: Invalid target GPI value in request: %u\n",
			target_gpi);
		return -EPERM;
	}

	/* Check that base and size are valid */
	if ((ULONG_MAX - base) < size) {
		VERBOSE("GPT: Transition request address overflow!\n");
		VERBOSE("      Base=0x%" PRIx64 "\n", base);
		VERBOSE("      Size=%lu\n", size);
		return -EINVAL;
	}

	/* Make sure base and size are valid */
	if (((base & (size - 1UL)) != 0UL) ||
	    ((base + size) >= GPT_PPS_ACTUAL_SIZE(firme_gpt_config.pps_bits))) {
		VERBOSE("GPT: Invalid granule transition address range!\n");
		VERBOSE("      Base=0x%" PRIx64 "\n", base);
		VERBOSE("      Size=%lu\n", size);
		return -EINVAL;
	}

	/* Get GPI info for next granule to transition. */
	res = get_gpi_params(base, &gpi_info);
	if (res != 0) {
		return res;
	}

	firme_gpt_lock(base);

	read_gpi(&gpi_info);

	/* Verify that transition of this granule is allowed. */
	if (!is_gpi_transition_permitted(src_sec_state, gpi_info.gpi,
					 target_gpi)) {
		VERBOSE("(%s) Sec state %u is not allowed to transition %u to %u!\n",
			__func__, src_sec_state, gpi_info.gpi, target_gpi);
		firme_gpt_unlock(base);
		return -EPERM;
	}

#if (RME_GPT_MAX_BLOCK != 0)
	/* Check for Contiguous descriptor */
	if ((gpi_info.gpt_l1_desc & GPT_L1_TYPE_CONT_DESC_MASK) ==
	    GPT_L1_TYPE_CONT_DESC) {
		shatter_block(base, &gpi_info, GPI_TO_DESC(gpi_info.gpi));
	}
#endif

	if (((target_gpi == GPT_GPI_NS) && (gpi_info.gpi == GPT_GPI_NSO)) ||
	    ((target_gpi == GPT_GPI_NSO) && (gpi_info.gpi == GPT_GPI_NS))) {
		/* Handle NS/NSO transition. */
		gpt_write_entry(base, target_gpi, &gpi_info);
	} else if ((target_gpi == GPT_GPI_NS) || (target_gpi == GPT_GPI_NSO)) {
		/* Handle undelegate transition. */
		gpt_undelegate(base, target_gpi, &gpi_info);
	} else {
		/* Handle delegate transition. */
		gpt_delegate(base, target_gpi, &gpi_info);
	}

#if (RME_GPT_MAX_BLOCK != 0)
	if (gpi_info.gpt_l1_desc == GPI_TO_DESC(target_gpi)) {
		/* Try to fuse */
		fuse_block(base, &gpi_info, GPI_TO_DESC(target_gpi));
	}
#endif

	firme_gpt_unlock(base);

	return 0;
}

uint32_t firme_gm_gpi_set(uint64_t base, uint64_t gcnt, uint64_t attrs,
			  uint64_t flags, uint64_t *gcnt_ret)
{
	uint32_t ret;
	uint8_t target_gpi;
	uint32_t src_sec_state = caller_sec_state(flags);

	assert(is_feat_rme_supported());

	if (gcnt == 0U) {
		return -EINVAL;
	}

	/* Extract target GPI value from attributes in x3. */
	target_gpi = (attrs >> FIRME_GM_GPI_SET_TGT_GPI_SHIFT) &
		FIRME_GM_GPI_SET_TGT_GPI_MASK;

	/* gpi set currently supports one granule  */
	ret = gm_gpi_set(base, target_gpi, src_sec_state);
	if (ret == 0) {
		*gcnt_ret = 1U;
	}

	return ret;
}

void firme_gm_gpi_init(void)
{
	const uint8_t pps_to_bits[] = { PPS_4GB_T, PPS_64GB_T, PPS_1TB_T,
		PPS_4TB_T, PPS_16TB_T, PPS_256TB_T, PPS_4PB_T };
	const uint8_t pgs_to_bits[] = { PGS_4KB_P, PGS_64KB_P, PGS_16KB_P };
	uint8_t pps_val;
	uint8_t pgs_val;

	assert(is_feat_rme_supported());

	pps_val = EXTRACT(GPCCR_PPS, read_gpccr_el3());
	pgs_val = EXTRACT(GPCCR_PGS, read_gpccr_el3());

	assert(pps_val < ARRAY_SIZE(pps_to_bits));
	assert(pgs_val < ARRAY_SIZE(pgs_to_bits));

	firme_gpt_config.pps_bits = pps_to_bits[pps_val];
	firme_gpt_config.pgs_bits = pgs_to_bits[pgs_val];

	firme_gpt_config.l1_cnt_2mb = GPT_L1_ENTRY_COUNT_2MB(firme_gpt_config.pgs_bits);

	/* Enable these GPIs in NSO/NSP transition policies used by GPI set */
	if (is_feat_rme_gdi_supported()) {
		firme_gpi_config[GPT_GPI_NS].policy[SMC_FROM_NON_SECURE] |=
			((1 << GPT_GPI_NSP) | (1 << GPT_GPI_SA));
		firme_gpi_config[GPT_GPI_NSO].policy[SMC_FROM_NON_SECURE] |=
			((1 << GPT_GPI_NSP) | (1 << GPT_GPI_SA));
		firme_gpi_config[GPT_GPI_NSP].policy[SMC_FROM_NON_SECURE] |=
			((1 << GPT_GPI_NS) | (1 << GPT_GPI_NSO));
		firme_gpi_config[GPT_GPI_SA].policy[SMC_FROM_NON_SECURE] |=
			((1 << GPT_GPI_NS) | (1 << GPT_GPI_NSO));
	}

	/* Enable NSO in NS transition policies used by FIRME GPI set. */
	if (is_feat_rme_gpc2_supported()) {
		firme_gpi_config[GPT_GPI_NS].policy[SMC_FROM_NON_SECURE] |=
			(1 << GPT_GPI_NSO);
		firme_gpi_config[GPT_GPI_NSO].policy[SMC_FROM_NON_SECURE] |=
			(1 << GPT_GPI_NS);
	}
}
