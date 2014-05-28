/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <xlat_tables.h>

/*******************************************************************************
 * This array holds the characteristics of the differences between the three
 * FVP platforms (Base, A53_A57 & Foundation). It will be populated during cold
 * boot at each boot stage by the primary before enabling the MMU (to allow cci
 * configuration) & used thereafter. Each BL will have its own copy to allow
 * independent operation.
 ******************************************************************************/
static unsigned long platform_config[CONFIG_LIMIT];

/*******************************************************************************
 * Macro generating the code for the function enabling the MMU in the given
 * exception level, assuming that the pagetables have already been created.
 *
 *   _el:		Exception level at which the function will run
 *   _tcr_extra:	Extra bits to set in the TCR register. This mask will
 *			be OR'ed with the default TCR value.
 *   _tlbi_fct:		Function to invalidate the TLBs at the current
 *			exception level
 ******************************************************************************/
#define DEFINE_ENABLE_MMU_EL(_el, _tcr_extra, _tlbi_fct)		\
	void enable_mmu_el##_el(void)					\
	{								\
		uint64_t mair, tcr, ttbr;				\
		uint32_t sctlr;						\
									\
		assert(IS_IN_EL(_el));					\
		assert((read_sctlr_el##_el() & SCTLR_M_BIT) == 0);	\
									\
		/* Set attributes in the right indices of the MAIR */	\
		mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);	\
		mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,		\
				ATTR_IWBWA_OWBWA_NTR_INDEX);		\
		write_mair_el##_el(mair);				\
									\
		/* Invalidate TLBs at the current exception level */	\
		_tlbi_fct();						\
									\
		/* Set TCR bits as well. */				\
		/* Inner & outer WBWA & shareable + T0SZ = 32 */	\
		tcr = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA |	\
			TCR_RGN_INNER_WBA | TCR_T0SZ_4GB;		\
		tcr |= _tcr_extra;					\
		write_tcr_el##_el(tcr);					\
									\
		/* Set TTBR bits as well */				\
		ttbr = (uint64_t) l1_xlation_table;			\
		write_ttbr0_el##_el(ttbr);				\
									\
		/* Ensure all translation table writes have drained */	\
		/* into memory, the TLB invalidation is complete, */	\
		/* and translation register writes are committed */	\
		/* before enabling the MMU */				\
		dsb();							\
		isb();							\
									\
		sctlr = read_sctlr_el##_el();				\
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT | SCTLR_I_BIT;	\
		sctlr |= SCTLR_A_BIT | SCTLR_C_BIT;			\
		write_sctlr_el##_el(sctlr);				\
									\
		/* Ensure the MMU enable takes effect immediately */	\
		isb();							\
	}

/* Define EL1 and EL3 variants of the function enabling the MMU */
DEFINE_ENABLE_MMU_EL(1, 0, tlbivmalle1)
DEFINE_ENABLE_MMU_EL(3, TCR_EL3_RES1, tlbialle3)

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t fvp_mmap[] = {
	{TZROM_BASE,	TZROM_BASE,	TZROM_SIZE,	MT_MEMORY | MT_RO | MT_SECURE },
	{TZDRAM_BASE,	TZDRAM_BASE,	TZDRAM_SIZE,	MT_MEMORY | MT_RW | MT_SECURE },
	{FLASH0_BASE,	FLASH0_BASE,	FLASH0_SIZE,	MT_MEMORY | MT_RO | MT_SECURE },
	{FLASH1_BASE,	FLASH1_BASE,	FLASH1_SIZE,	MT_MEMORY | MT_RO | MT_SECURE },
	{VRAM_BASE,	VRAM_BASE,	VRAM_SIZE,	MT_MEMORY | MT_RW | MT_SECURE },
	{DEVICE0_BASE,	DEVICE0_BASE,	DEVICE0_SIZE,	MT_DEVICE | MT_RW | MT_SECURE },
	{NSRAM_BASE,	NSRAM_BASE,	NSRAM_SIZE,	MT_MEMORY | MT_RW | MT_NS },
	{DEVICE1_BASE,	DEVICE1_BASE,	DEVICE1_SIZE,	MT_DEVICE | MT_RW | MT_SECURE },
	/* 2nd GB as device for now...*/
	{0x40000000,	0x40000000,	0x40000000,	MT_DEVICE | MT_RW | MT_SECURE },
	{DRAM_BASE,	DRAM_BASE,	DRAM_SIZE,	MT_MEMORY | MT_RW | MT_NS },
	{0}
};

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void configure_mmu_el##_el(meminfo_t *mem_layout,		\
				   unsigned long ro_start,		\
				   unsigned long ro_limit,		\
				   unsigned long coh_start,		\
				   unsigned long coh_limit)		\
	{								\
		mmap_add_region(mem_layout->total_base,			\
				mem_layout->total_base,			\
				mem_layout->total_size,			\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(fvp_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el##_el();					\
	}

/* Define EL1 and EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(1)
DEFINE_CONFIGURE_MMU_EL(3)

/* Simple routine which returns a configuration variable value */
unsigned long platform_get_cfgvar(unsigned int var_id)
{
	assert(var_id < CONFIG_LIMIT);
	return platform_config[var_id];
}

/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation FVP
 * models and the two flavours of the Base FVP models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
int platform_config_setup(void)
{
	unsigned int rev, hbi, bld, arch, sys_id, midr_pn;

	sys_id = mmio_read_32(VE_SYSREGS_BASE + V2M_SYS_ID);
	rev = (sys_id >> SYS_ID_REV_SHIFT) & SYS_ID_REV_MASK;
	hbi = (sys_id >> SYS_ID_HBI_SHIFT) & SYS_ID_HBI_MASK;
	bld = (sys_id >> SYS_ID_BLD_SHIFT) & SYS_ID_BLD_MASK;
	arch = (sys_id >> SYS_ID_ARCH_SHIFT) & SYS_ID_ARCH_MASK;

	if ((rev != REV_FVP) || (arch != ARCH_MODEL))
		panic();

	/*
	 * The build field in the SYS_ID tells which variant of the GIC
	 * memory is implemented by the model.
	 */
	switch (bld) {
	case BLD_GIC_VE_MMAP:
		platform_config[CONFIG_GICD_ADDR] = VE_GICD_BASE;
		platform_config[CONFIG_GICC_ADDR] = VE_GICC_BASE;
		platform_config[CONFIG_GICH_ADDR] = VE_GICH_BASE;
		platform_config[CONFIG_GICV_ADDR] = VE_GICV_BASE;
		break;
	case BLD_GIC_A53A57_MMAP:
		platform_config[CONFIG_GICD_ADDR] = BASE_GICD_BASE;
		platform_config[CONFIG_GICC_ADDR] = BASE_GICC_BASE;
		platform_config[CONFIG_GICH_ADDR] = BASE_GICH_BASE;
		platform_config[CONFIG_GICV_ADDR] = BASE_GICV_BASE;
		break;
	default:
		assert(0);
	}

	/*
	 * The hbi field in the SYS_ID is 0x020 for the Base FVP & 0x010
	 * for the Foundation FVP.
	 */
	switch (hbi) {
	case HBI_FOUNDATION:
		platform_config[CONFIG_MAX_AFF0] = 4;
		platform_config[CONFIG_MAX_AFF1] = 1;
		platform_config[CONFIG_CPU_SETUP] = 0;
		platform_config[CONFIG_BASE_MMAP] = 0;
		platform_config[CONFIG_HAS_CCI] = 0;
		platform_config[CONFIG_HAS_TZC] = 0;
		break;
	case HBI_FVP_BASE:
		midr_pn = (read_midr() >> MIDR_PN_SHIFT) & MIDR_PN_MASK;
		if ((midr_pn == MIDR_PN_A57) || (midr_pn == MIDR_PN_A53))
			platform_config[CONFIG_CPU_SETUP] = 1;
		else
			platform_config[CONFIG_CPU_SETUP] = 0;

		platform_config[CONFIG_MAX_AFF0] = 4;
		platform_config[CONFIG_MAX_AFF1] = 2;
		platform_config[CONFIG_BASE_MMAP] = 1;
		platform_config[CONFIG_HAS_CCI] = 1;
		platform_config[CONFIG_HAS_TZC] = 1;
		break;
	default:
		assert(0);
	}

	return 0;
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return NS_IMAGE_OFFSET;
}

uint64_t plat_get_syscnt_freq(void)
{
	uint64_t counter_base_frequency;

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	assert(counter_base_frequency != 0);

	return counter_base_frequency;
}
