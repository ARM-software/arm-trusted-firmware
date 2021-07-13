/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/gpt/gpt.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#if !ENABLE_RME
#error "ENABLE_RME must be enabled to use the GPT library."
#endif

typedef struct {
	uintptr_t plat_gpt_l0_base;
	uintptr_t plat_gpt_l1_base;
	size_t plat_gpt_l0_size;
	size_t plat_gpt_l1_size;
	unsigned int plat_gpt_pps;
	unsigned int plat_gpt_pgs;
	unsigned int plat_gpt_l0gptsz;
} gpt_config_t;

gpt_config_t gpt_config;

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
/* Helper function that cleans the data cache only if it is enabled. */
static inline
	void gpt_clean_dcache_range(uintptr_t addr, size_t size)
{
	if ((read_sctlr_el3() & SCTLR_C_BIT) != 0U) {
		clean_dcache_range(addr, size);
	}
}

/* Helper function that invalidates the data cache only if it is enabled. */
static inline
	void gpt_inv_dcache_range(uintptr_t addr, size_t size)
{
	if ((read_sctlr_el3() & SCTLR_C_BIT) != 0U) {
		inv_dcache_range(addr, size);
	}
}
#endif

typedef struct l1_gpt_attr_desc {
	size_t t_sz;		/** Table size */
	size_t g_sz;		/** Granularity size */
	unsigned int p_val;	/** Associated P value */
} l1_gpt_attr_desc_t;

/*
 * Lookup table to find out the size in bytes of the L1 tables as well
 * as the index mask, given the Width of Physical Granule Size (PGS).
 * L1 tables are indexed by PA[29:p+4], being 'p' the width in bits of the
 * aforementioned Physical Granule Size.
 */
static const l1_gpt_attr_desc_t l1_gpt_attr_lookup[] = {
	[GPCCR_PGS_4K]  = {U(1) << U(17),  /* 16384B x 64bit entry = 128KB */
			   PAGE_SIZE_4KB,  /* 4KB Granularity  */
			   U(12)},
	[GPCCR_PGS_64K] = {U(1) << U(13),  /* Table size = 8KB  */
			   PAGE_SIZE_64KB, /* 64KB Granularity  */
			  U(16)},
	[GPCCR_PGS_16K] = {U(1) << U(15),  /* Table size = 32KB */
			   PAGE_SIZE_16KB, /* 16KB Granularity  */
			   U(14)}
};

typedef struct l0_gpt_attr_desc {
	size_t sz;
	unsigned int t_val_mask;
} l0_gpt_attr_desc_t;

/*
 * Lookup table to find out the size in bytes of the L0 table as well
 * as the index mask, given the Protected Physical Address Size (PPS).
 * L0 table is indexed by PA[t-1:30], being 't' the size in bits
 * of the aforementioned Protected Physical Address Size.
 */
static const l0_gpt_attr_desc_t  l0_gpt_attr_lookup[] = {

	[GPCCR_PPS_4GB]   = {U(1) << U(5),   /* 4 x 64 bit entry = 32 bytes */
			     0x3},	     /* Bits[31:30]   */

	[GPCCR_PPS_64GB]  = {U(1) << U(9),   /* 512 bytes     */
			     0x3f},	     /* Bits[35:30]   */

	[GPCCR_PPS_1TB]   = {U(1) << U(13),  /* 8KB	      */
			     0x3ff},	     /* Bits[39:30]   */

	[GPCCR_PPS_4TB]   = {U(1) << U(15),  /* 32KB	      */
			     0xfff},	     /* Bits[41:30]   */

	[GPCCR_PPS_16TB]  = {U(1) << U(17),  /* 128KB	      */
			     0x3fff},	     /* Bits[43:30]   */

	[GPCCR_PPS_256TB] = {U(1) << U(21),  /* 2MB	      */
			     0x3ffff},	     /* Bits[47:30]   */

	[GPCCR_PPS_4PB]   = {U(1) << U(25),  /* 32MB	      */
			     0x3fffff},	     /* Bits[51:30]   */

};

static unsigned int get_l1_gpt_index(unsigned int pgs, uintptr_t pa)
{
	unsigned int l1_gpt_arr_idx;

	/*
	 * Mask top 2 bits to obtain the 30 bits required to
	 * generate the L1 GPT index
	 */
	l1_gpt_arr_idx = (unsigned int)(pa & L1_GPT_INDEX_MASK);

	/* Shift by 'p' value + 4 to obtain the index */
	l1_gpt_arr_idx >>= (l1_gpt_attr_lookup[pgs].p_val + 4);

	return l1_gpt_arr_idx;
}

unsigned int plat_is_my_cpu_primary(void);

/* The granule partition tables can only be configured on BL2 */
#ifdef IMAGE_BL2

/* Global to keep track of next available index in array of L1 GPTs */
static unsigned int l1_gpt_mem_avlbl_index;

static int validate_l0_gpt_params(gpt_init_params_t *params)
{
	/* Only 1GB of address space per L0 entry is allowed */
	if (params->l0gptsz != GPCCR_L0GPTSZ_30BITS) {
		WARN("Invalid L0GPTSZ %u.\n", params->l0gptsz);
	}

	/* Only 4K granule is supported for now */
	if (params->pgs != GPCCR_PGS_4K) {
		WARN("Invalid GPT PGS %u.\n", params->pgs);
		return -EINVAL;
	}

	/* Only 4GB of protected physical address space is supported for now */
	if (params->pps != GPCCR_PPS_4GB) {
		WARN("Invalid GPT PPS %u.\n", params->pps);
		return -EINVAL;
	}

	/* Check if GPT base address is aligned with the system granule */
	if (!IS_PAGE_ALIGNED(params->l0_mem_base)) {
		ERROR("Unaligned L0 GPT base address.\n");
		return -EFAULT;
	}

	/* Check if there is enough memory for L0 GPTs */
	if (params->l0_mem_size < l0_gpt_attr_lookup[params->pps].sz) {
		ERROR("Inadequate memory for L0 GPTs. ");
		ERROR("Expected 0x%lx bytes. Got 0x%lx bytes\n",
		     l0_gpt_attr_lookup[params->pps].sz,
		     params->l0_mem_size);
		return -ENOMEM;
	}

	return 0;
}

/*
 * A L1 GPT is required if any one of the following conditions is true:
 *
 * - The base address is not 1GB aligned
 * - The size of the memory region is not a multiple of 1GB
 * - A L1 GPT has been explicitly requested (attrs == PAS_REG_DESC_TYPE_TBL)
 *
 * This function:
 * - iterates over all the PAS regions to determine whether they
 *   will need a 2 stage look up (and therefore a L1 GPT will be required) or
 *   if it would be enough with a single level lookup table.
 * - Updates the attr field of the PAS regions.
 * - Returns the total count of L1 tables needed.
 *
 * In the future wwe should validate that the PAS range does not exceed the
 * configured PPS. (and maybe rename this function as it is validating PAS
 * regions).
 */
static unsigned int update_gpt_type(pas_region_t *pas_regions,
				    unsigned int pas_region_cnt)
{
	unsigned int idx, cnt = 0U;

	for (idx = 0U; idx < pas_region_cnt; idx++) {
		if (PAS_REG_DESC_TYPE(pas_regions[idx].attrs) ==
						PAS_REG_DESC_TYPE_TBL) {
			cnt++;
			continue;
		}
		if (!(IS_1GB_ALIGNED(pas_regions[idx].base_pa) &&
			IS_1GB_ALIGNED(pas_regions[idx].size))) {

			/* Current region will need L1 GPTs. */
			assert(PAS_REG_DESC_TYPE(pas_regions[idx].attrs)
						== PAS_REG_DESC_TYPE_ANY);

			pas_regions[idx].attrs =
				GPT_DESC_ATTRS(PAS_REG_DESC_TYPE_TBL,
					PAS_REG_GPI(pas_regions[idx].attrs));
			cnt++;
			continue;
		}

		/* The PAS can be mapped on a one stage lookup table */
		assert(PAS_REG_DESC_TYPE(pas_regions[idx].attrs) !=
							PAS_REG_DESC_TYPE_TBL);

		pas_regions[idx].attrs = GPT_DESC_ATTRS(PAS_REG_DESC_TYPE_BLK,
					PAS_REG_GPI(pas_regions[idx].attrs));
	}

	return cnt;
}

static int validate_l1_gpt_params(gpt_init_params_t *params,
				  unsigned int l1_gpt_cnt)
{
	size_t l1_gpt_sz, l1_gpt_mem_sz;

	/* Check if the granularity is supported */
	assert(xlat_arch_is_granule_size_supported(
					l1_gpt_attr_lookup[params->pgs].g_sz));


	/* Check if naturally aligned L1 GPTs can be created */
	l1_gpt_sz = l1_gpt_attr_lookup[params->pgs].g_sz;
	if (params->l1_mem_base & (l1_gpt_sz - 1)) {
		WARN("Unaligned L1 GPT base address.\n");
		return -EFAULT;
	}

	/* Check if there is enough memory for L1 GPTs */
	l1_gpt_mem_sz = l1_gpt_cnt * l1_gpt_sz;
	if (params->l1_mem_size < l1_gpt_mem_sz) {
		WARN("Inadequate memory for L1 GPTs. ");
		WARN("Expected 0x%lx bytes. Got 0x%lx bytes\n",
		     l1_gpt_mem_sz, params->l1_mem_size);
		return -ENOMEM;
	}

	INFO("Requested 0x%lx bytes for L1 GPTs.\n", l1_gpt_mem_sz);
	return 0;
}

/*
 * Helper function to determine if the end physical address lies in the same GB
 * as the current physical address. If true, the end physical address is
 * returned else, the start address of the next GB is returned.
 */
static uintptr_t get_l1_gpt_end_pa(uintptr_t cur_pa, uintptr_t end_pa)
{
	uintptr_t cur_gb, end_gb;

	cur_gb = cur_pa >> ONE_GB_SHIFT;
	end_gb = end_pa >> ONE_GB_SHIFT;

	assert(cur_gb <= end_gb);

	if (cur_gb == end_gb) {
		return end_pa;
	}

	return (cur_gb + 1) << ONE_GB_SHIFT;
}

static void generate_l0_blk_desc(gpt_init_params_t *params,
				 unsigned int idx)
{
	uint64_t gpt_desc;
	uintptr_t end_addr;
	unsigned int end_idx, start_idx;
	pas_region_t *pas = params->pas_regions + idx;
	uint64_t *l0_gpt_arr = (uint64_t *)params->l0_mem_base;

	/* Create the GPT Block descriptor for this PAS region */
	gpt_desc = GPT_BLK_DESC;
	gpt_desc |= PAS_REG_GPI(pas->attrs)
		    << GPT_BLOCK_DESC_GPI_VAL_SHIFT;

	/* Start index of this region in L0 GPTs */
	start_idx = pas->base_pa >> ONE_GB_SHIFT;

	/*
	 * Determine number of L0 GPT descriptors covered by
	 * this PAS region and use the count to populate these
	 * descriptors.
	 */
	end_addr = pas->base_pa + pas->size;
	assert(end_addr \
	       <= (ULL(l0_gpt_attr_lookup[params->pps].t_val_mask + 1)) << 30);
	end_idx = end_addr >> ONE_GB_SHIFT;

	for (; start_idx < end_idx; start_idx++) {
		l0_gpt_arr[start_idx] = gpt_desc;
		INFO("L0 entry (BLOCK) index %u [%p]: GPI = 0x%llx (0x%llx)\n",
			start_idx, &l0_gpt_arr[start_idx],
			(gpt_desc >> GPT_BLOCK_DESC_GPI_VAL_SHIFT) &
			GPT_L1_INDEX_MASK, l0_gpt_arr[start_idx]);
	}
}

static void generate_l0_tbl_desc(gpt_init_params_t *params,
				 unsigned int idx)
{
	uint64_t gpt_desc = 0U, *l1_gpt_arr;
	uintptr_t start_pa, end_pa, cur_pa, next_pa;
	unsigned int start_idx, l1_gpt_idx;
	unsigned int p_val, gran_sz;
	pas_region_t *pas = params->pas_regions + idx;
	uint64_t *l0_gpt_base = (uint64_t *)params->l0_mem_base;
	uint64_t *l1_gpt_base = (uint64_t *)params->l1_mem_base;

	start_pa = pas->base_pa;
	end_pa = start_pa + pas->size;
	p_val = l1_gpt_attr_lookup[params->pgs].p_val;
	gran_sz = 1 << p_val;

	/*
	 * end_pa cannot be larger than the maximum protected physical memory.
	 */
	assert(((1ULL<<30) << l0_gpt_attr_lookup[params->pps].t_val_mask)
								 > end_pa);

	for (cur_pa = start_pa; cur_pa < end_pa;) {
		/*
		 * Determine the PA range that will be covered
		 * in this loop iteration.
		 */
		next_pa = get_l1_gpt_end_pa(cur_pa, end_pa);

		INFO("PAS[%u]: start: 0x%lx, end: 0x%lx, next_pa: 0x%lx.\n",
		     idx, cur_pa, end_pa, next_pa);

		/* Index of this PA in L0 GPTs */
		start_idx = cur_pa >> ONE_GB_SHIFT;

		/*
		 * If cur_pa is on a 1GB boundary then determine
		 * the base address of next available L1 GPT
		 * memory region
		 */
		if (IS_1GB_ALIGNED(cur_pa)) {
			l1_gpt_arr = (uint64_t *)((uint64_t)l1_gpt_base +
					(l1_gpt_attr_lookup[params->pgs].t_sz *
					 l1_gpt_mem_avlbl_index));

			assert(l1_gpt_arr <
			       (l1_gpt_base + params->l1_mem_size));

			/* Create the L0 GPT descriptor for this PAS region */
			gpt_desc = GPT_TBL_DESC |
				   ((uintptr_t)l1_gpt_arr
				    & GPT_TBL_DESC_ADDR_MASK);

			l0_gpt_base[start_idx] = gpt_desc;

			/*
			 * Update index to point to next available L1
			 * GPT memory region
			 */
			l1_gpt_mem_avlbl_index++;
		} else {
			/* Use the existing L1 GPT */
			l1_gpt_arr = (uint64_t *)(l0_gpt_base[start_idx]
							& ~((1U<<12) - 1U));
		}

		INFO("L0 entry (TABLE) index %u [%p] ==> L1 Addr 0x%llx (0x%llx)\n",
			start_idx, &l0_gpt_base[start_idx],
			(unsigned long long)(l1_gpt_arr),
			l0_gpt_base[start_idx]);

		/*
		 * Fill up L1 GPT entries between these two
		 * addresses.
		 */
		for (; cur_pa < next_pa; cur_pa += gran_sz) {
			unsigned int gpi_idx, gpi_idx_shift;

			/* Obtain index of L1 GPT entry */
			l1_gpt_idx = get_l1_gpt_index(params->pgs, cur_pa);

			/*
			 * Obtain index of GPI in L1 GPT entry
			 * (i = PA[p_val+3:p_val])
			 */
			gpi_idx = (cur_pa >> p_val) & GPT_L1_INDEX_MASK;

			/*
			 * Shift by index * 4 to reach correct
			 * GPI entry in L1 GPT descriptor.
			 * GPI = gpt_desc[(4*idx)+3:(4*idx)]
			 */
			gpi_idx_shift = gpi_idx << 2;

			gpt_desc = l1_gpt_arr[l1_gpt_idx];

			/* Clear existing GPI encoding */
			gpt_desc &= ~(GPT_L1_INDEX_MASK << gpi_idx_shift);

			/* Set the GPI encoding */
			gpt_desc |= ((uint64_t)PAS_REG_GPI(pas->attrs)
				     << gpi_idx_shift);

			l1_gpt_arr[l1_gpt_idx] = gpt_desc;

			if (gpi_idx == 15U) {
				VERBOSE("\tEntry %u [%p] = 0x%llx\n",
					l1_gpt_idx,
					&l1_gpt_arr[l1_gpt_idx], gpt_desc);
			}
		}
	}
}

static void create_gpt(gpt_init_params_t *params)
{
	unsigned int idx;
	pas_region_t *pas_regions = params->pas_regions;

	INFO("pgs = 0x%x, pps = 0x%x, l0gptsz = 0x%x\n",
	     params->pgs, params->pps, params->l0gptsz);
	INFO("pas_region_cnt = 0x%x L1 base = 0x%lx, L1 sz = 0x%lx\n",
	     params->pas_count, params->l1_mem_base, params->l1_mem_size);

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	gpt_inv_dcache_range(params->l0_mem_base, params->l0_mem_size);
	gpt_inv_dcache_range(params->l1_mem_base, params->l1_mem_size);
#endif

	for (idx = 0U; idx < params->pas_count; idx++) {

		INFO("PAS[%u]: base 0x%llx, sz 0x%lx, GPI 0x%x, type 0x%x\n",
		     idx, pas_regions[idx].base_pa, pas_regions[idx].size,
		     PAS_REG_GPI(pas_regions[idx].attrs),
		     PAS_REG_DESC_TYPE(pas_regions[idx].attrs));

		/* Check if a block or table descriptor is required */
		if (PAS_REG_DESC_TYPE(pas_regions[idx].attrs) ==
		     PAS_REG_DESC_TYPE_BLK) {
			generate_l0_blk_desc(params, idx);

		} else {
			generate_l0_tbl_desc(params, idx);
		}
	}

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	gpt_clean_dcache_range(params->l0_mem_base, params->l0_mem_size);
	gpt_clean_dcache_range(params->l1_mem_base, params->l1_mem_size);
#endif

	/* Make sure that all the entries are written to the memory. */
	dsbishst();
}

#endif /* IMAGE_BL2 */

int gpt_init(gpt_init_params_t *params)
{
#ifdef IMAGE_BL2
	unsigned int l1_gpt_cnt;
	int ret;
#endif
	/* Validate arguments */
	assert(params != NULL);
	assert(params->pgs <= GPCCR_PGS_16K);
	assert(params->pps <= GPCCR_PPS_4PB);
	assert(params->l0_mem_base != (uintptr_t)0);
	assert(params->l0_mem_size > 0U);
	assert(params->l1_mem_base != (uintptr_t)0);
	assert(params->l1_mem_size > 0U);

#ifdef IMAGE_BL2
	/*
	 * The Granule Protection Tables are initialised only in BL2.
	 * BL31 is not allowed to initialise them again in case
	 * these are modified by any other image loaded by BL2.
	 */
	assert(params->pas_regions != NULL);
	assert(params->pas_count > 0U);

	ret = validate_l0_gpt_params(params);
	if (ret < 0) {

		return ret;
	}

	/* Check if L1 GPTs are required and how many. */
	l1_gpt_cnt = update_gpt_type(params->pas_regions,
				     params->pas_count);
	INFO("%u L1 GPTs requested.\n", l1_gpt_cnt);

	if (l1_gpt_cnt > 0U) {
		ret = validate_l1_gpt_params(params, l1_gpt_cnt);
		if (ret < 0) {
			return ret;
		}
	}

	create_gpt(params);
#else
	/* If running in BL31, only primary CPU can initialise GPTs */
	assert(plat_is_my_cpu_primary() == 1U);

	/*
	 * If the primary CPU is calling this function from BL31
	 * we expect that the tables are aready initialised from
	 * BL2 and GPCCR_EL3 is already configured with
	 * Granule Protection Check Enable bit set.
	 */
	assert((read_gpccr_el3() & GPCCR_GPC_BIT) != 0U);
#endif /* IMAGE_BL2 */

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	gpt_inv_dcache_range((uintptr_t)&gpt_config, sizeof(gpt_config));
#endif
	gpt_config.plat_gpt_l0_base = params->l0_mem_base;
	gpt_config.plat_gpt_l1_base = params->l1_mem_base;
	gpt_config.plat_gpt_l0_size = params->l0_mem_size;
	gpt_config.plat_gpt_l1_size = params->l1_mem_size;

	/* Backup the parameters used to configure GPCCR_EL3 on every PE. */
	gpt_config.plat_gpt_pgs = params->pgs;
	gpt_config.plat_gpt_pps = params->pps;
	gpt_config.plat_gpt_l0gptsz = params->l0gptsz;

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	gpt_clean_dcache_range((uintptr_t)&gpt_config, sizeof(gpt_config));
#endif

	return 0;
}

void gpt_enable(void)
{
	u_register_t gpccr_el3;

	/* Invalidate any stale TLB entries */
	tlbipaallos();

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	gpt_inv_dcache_range((uintptr_t)&gpt_config, sizeof(gpt_config));
#endif

#ifdef IMAGE_BL2
	/*
	 * Granule tables must be initialised before enabling
	 * granule protection.
	 */
	assert(gpt_config.plat_gpt_l0_base != (uintptr_t)NULL);
#endif
	write_gptbr_el3(gpt_config.plat_gpt_l0_base >> GPTBR_BADDR_VAL_SHIFT);

	/* GPCCR_EL3.L0GPTSZ */
	gpccr_el3 = SET_GPCCR_L0GPTSZ(gpt_config.plat_gpt_l0gptsz);

	/* GPCCR_EL3.PPS */
	gpccr_el3 |= SET_GPCCR_PPS(gpt_config.plat_gpt_pps);

	/* GPCCR_EL3.PGS */
	gpccr_el3 |= SET_GPCCR_PGS(gpt_config.plat_gpt_pgs);

	/* Set shareability attribute to Outher Shareable */
	gpccr_el3 |= SET_GPCCR_SH(GPCCR_SH_OS);

	/* Outer and Inner cacheability set to Normal memory, WB, RA, WA. */
	gpccr_el3 |= SET_GPCCR_ORGN(GPCCR_ORGN_WB_RA_WA);
	gpccr_el3 |= SET_GPCCR_IRGN(GPCCR_IRGN_WB_RA_WA);

	/* Enable GPT */
	gpccr_el3 |= GPCCR_GPC_BIT;

	write_gpccr_el3(gpccr_el3);
	dsbsy();

	VERBOSE("Granule Protection Checks enabled\n");
}

void gpt_disable(void)
{
	u_register_t gpccr_el3 = read_gpccr_el3();

	write_gpccr_el3(gpccr_el3 &= ~GPCCR_GPC_BIT);
	dsbsy();
}

#ifdef IMAGE_BL31

/*
 * Each L1 descriptor is protected by 1 spinlock. The number of descriptors is
 * equal to the size of the total protected memory area divided by the size of
 * protected memory area covered by each descriptor.
 *
 * The size of memory covered by each descriptor is the 'size of the granule' x
 * 'number of granules' in a descriptor. The former is PLAT_ARM_GPT_PGS and
 * latter is always 16.
 */
static spinlock_t gpt_lock;

static unsigned int get_l0_gpt_index(unsigned int pps, uint64_t pa)
{
	unsigned int idx;

	/* Get the index into the L0 table */
	idx = pa >> ONE_GB_SHIFT;

	/* Check if the pa lies within the PPS */
	if (idx & ~(l0_gpt_attr_lookup[pps].t_val_mask)) {
		WARN("Invalid address 0x%llx.\n", pa);
		return -EINVAL;
	}

	return idx;
}

int gpt_transition_pas(uint64_t pa,
			unsigned int src_sec_state,
			unsigned int target_pas)
{
	int idx;
	unsigned int idx_shift;
	unsigned int gpi;
	uint64_t gpt_l1_desc;
	uint64_t *gpt_l1_addr, *gpt_addr;

	/*
	 * Check if caller is allowed to transition the granule's PAS.
	 *
	 * - Secure world caller can only request S <-> NS transitions on a
	 *   granule that is already in either S or NS PAS.
	 *
	 * - Realm world caller can only request R <-> NS transitions on a
	 *   granule that is already in either R or NS PAS.
	 */
	if (src_sec_state == SMC_FROM_REALM) {
		if ((target_pas != GPI_REALM) && (target_pas != GPI_NS)) {
			WARN("Invalid caller (%s) and PAS (%d) combination.\n",
			     "realm world", target_pas);
			return -EINVAL;
		}
	} else if (src_sec_state == SMC_FROM_SECURE) {
		if ((target_pas != GPI_SECURE) && (target_pas != GPI_NS)) {
			WARN("Invalid caller (%s) and PAS (%d) combination.\n",
			     "secure world", target_pas);
			return -EINVAL;
		}
	} else {
		WARN("Invalid caller security state 0x%x\n", src_sec_state);
		return -EINVAL;
	}

	/* Obtain the L0 GPT address. */
	gpt_addr = (uint64_t *)gpt_config.plat_gpt_l0_base;

	/* Validate physical address and obtain index into L0 GPT table */
	idx = get_l0_gpt_index(gpt_config.plat_gpt_pps, pa);
	if (idx < 0U) {
		return idx;
	}

	VERBOSE("PA 0x%llx, L0 base addr 0x%llx, L0 index %u\n",
					pa, (uint64_t)gpt_addr, idx);

	/* Obtain the L0 descriptor */
	gpt_l1_desc = gpt_addr[idx];

	/*
	 * Check if it is a table descriptor. Granule transition only applies to
	 * memory ranges for which L1 tables were created at boot time. So there
	 * is no possibility of splitting and coalescing tables.
	 */
	if ((gpt_l1_desc & GPT_L1_INDEX_MASK) != GPT_TBL_DESC) {
		WARN("Invalid address 0x%llx.\n", pa);
		return -EPERM;
	}

	/* Obtain the L1 table address from L0 descriptor. */
	gpt_l1_addr = (uint64_t *)(gpt_l1_desc & ~(0xFFF));

	/* Obtain the index into the L1 table */
	idx = get_l1_gpt_index(gpt_config.plat_gpt_pgs, pa);

	VERBOSE("L1 table base addr 0x%llx, L1 table index %u\n", (uint64_t)gpt_l1_addr, idx);

	/* Lock access to the granule */
	spin_lock(&gpt_lock);

	/* Obtain the L1 descriptor */
	gpt_l1_desc = gpt_l1_addr[idx];

	/* Obtain the shift for GPI in L1 GPT entry */
	idx_shift = (pa >> 12) & GPT_L1_INDEX_MASK;
	idx_shift <<= 2;

	/* Obtain the current GPI encoding for this PA */
	gpi = (gpt_l1_desc >> idx_shift) & GPT_L1_INDEX_MASK;

	if (src_sec_state == SMC_FROM_REALM) {
		/*
		 * Realm world is only allowed to transition a NS or Realm world
		 * granule.
		 */
		if ((gpi != GPI_REALM) && (gpi != GPI_NS)) {
			WARN("Invalid transition request from %s.\n",
			     "realm world");
			spin_unlock(&gpt_lock);
			return -EPERM;
		}
	} else if (src_sec_state == SMC_FROM_SECURE) {
		/*
		 * Secure world is only allowed to transition a NS or Secure world
		 * granule.
		 */
		if ((gpi != GPI_SECURE) && (gpi != GPI_NS)) {
			WARN("Invalid transition request from %s.\n",
			     "secure world");
			spin_unlock(&gpt_lock);
			return -EPERM;
		}
	}
	/* We don't need an else here since we already handle that above. */

	VERBOSE("L1 table desc 0x%llx before mod \n", gpt_l1_desc);

	/* Clear existing GPI encoding */
	gpt_l1_desc &= ~(GPT_L1_INDEX_MASK << idx_shift);

	/* Transition the granule to the new PAS */
	gpt_l1_desc |= ((uint64_t)target_pas << idx_shift);

	/* Update the L1 GPT entry */
	gpt_l1_addr[idx] = gpt_l1_desc;

	VERBOSE("L1 table desc 0x%llx after mod \n", gpt_l1_desc);

	/* Make sure change is propagated to other CPUs. */
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	gpt_clean_dcache_range((uintptr_t)&gpt_addr[idx], sizeof(uint64_t));
#endif

	gpt_tlbi_by_pa(pa, PAGE_SIZE_4KB);

	/* Make sure that all the entries are written to the memory. */
	dsbishst();

	/* Unlock access to the granule */
	spin_unlock(&gpt_lock);

	return 0;
}

#endif /* IMAGE_BL31 */
