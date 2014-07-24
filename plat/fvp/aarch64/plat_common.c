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
 * Enable the MMU assuming that the pagetables have already been created
 *******************************************************************************/
void enable_mmu()
{
	unsigned long mair, tcr, ttbr, sctlr;
	unsigned long current_el = read_current_el();

	/* Set the attributes in the right indices of the MAIR */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
				  ATTR_IWBWA_OWBWA_NTR_INDEX);

	/*
	 * Set TCR bits as well. Inner & outer WBWA & shareable + T0SZ = 32
	 */
	tcr = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA |
		  TCR_RGN_INNER_WBA | TCR_T0SZ_4GB;

	/* Set TTBR bits as well */
	ttbr = (unsigned long) l1_xlation_table;

	if (GET_EL(current_el) == MODE_EL3) {
		assert((read_sctlr_el3() & SCTLR_M_BIT) == 0);

		write_mair_el3(mair);
		tcr |= TCR_EL3_RES1;
		/* Invalidate EL3 TLBs */
		tlbialle3();

		write_tcr_el3(tcr);
		write_ttbr0_el3(ttbr);

		/* ensure all translation table writes have drained into memory,
		 * the TLB invalidation is complete, and translation register
		 * writes are committed before enabling the MMU
		 */
		dsb();
		isb();

		sctlr = read_sctlr_el3();
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT | SCTLR_I_BIT;
		sctlr |= SCTLR_A_BIT | SCTLR_C_BIT;
		write_sctlr_el3(sctlr);
	} else {
		assert((read_sctlr_el1() & SCTLR_M_BIT) == 0);

		write_mair_el1(mair);
		/* Invalidate EL1 TLBs */
		tlbivmalle1();

		write_tcr_el1(tcr);
		write_ttbr0_el1(ttbr);

		/* ensure all translation table writes have drained into memory,
		 * the TLB invalidation is complete, and translation register
		 * writes are committed before enabling the MMU
		 */
		dsb();
		isb();

		sctlr = read_sctlr_el1();
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT | SCTLR_I_BIT;
		sctlr |= SCTLR_A_BIT | SCTLR_C_BIT;
		write_sctlr_el1(sctlr);
	}
	/* ensure the MMU enable takes effect immediately */
	isb();

	return;
}

void disable_mmu(void)
{
	unsigned long sctlr;
	unsigned long current_el = read_current_el();

	if (GET_EL(current_el) == MODE_EL3) {
		sctlr = read_sctlr_el3();
		sctlr = sctlr & ~(SCTLR_M_BIT | SCTLR_C_BIT);
		write_sctlr_el3(sctlr);
	} else {
		sctlr = read_sctlr_el1();
		sctlr = sctlr & ~(SCTLR_M_BIT | SCTLR_C_BIT);
		write_sctlr_el1(sctlr);
	}
	/* ensure the MMU disable takes effect immediately */
	isb();

	/* Flush the caches */
	dcsw_op_all(DCCISW);

	return;
}

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to to
 * configure_mmu() will give the available subset of that,
 */
const mmap_region_t fvp_mmap[] = {
	{ TZROM_BASE,	TZROM_SIZE,	MT_MEMORY | MT_RO | MT_SECURE },
	{ TZDRAM_BASE,	TZDRAM_SIZE,	MT_MEMORY | MT_RW | MT_SECURE },
	{ FLASH0_BASE,	FLASH0_SIZE,	MT_MEMORY | MT_RO | MT_SECURE },
	{ FLASH1_BASE,	FLASH1_SIZE,	MT_MEMORY | MT_RO | MT_SECURE },
	{ VRAM_BASE,	VRAM_SIZE,	MT_MEMORY | MT_RW | MT_SECURE },
	{ DEVICE0_BASE,	DEVICE0_SIZE,	MT_DEVICE | MT_RW | MT_SECURE },
	{ NSRAM_BASE,	NSRAM_SIZE,	MT_MEMORY | MT_RW | MT_NS },
	{ DEVICE1_BASE,	DEVICE1_SIZE,	MT_DEVICE | MT_RW | MT_SECURE },
	/* 2nd GB as device for now...*/
	{ 0x40000000,	0x40000000,	MT_DEVICE | MT_RW | MT_SECURE },
	{ DRAM_BASE,	DRAM_SIZE,	MT_MEMORY | MT_RW | MT_NS },
	{0}
};

/*******************************************************************************
 * Setup the pagetables as per the platform memory map & initialize the mmu
 *******************************************************************************/
void configure_mmu(meminfo_t *mem_layout,
		   unsigned long ro_start,
		   unsigned long ro_limit,
		   unsigned long coh_start,
		   unsigned long coh_limit)
{
	mmap_add_region(mem_layout->total_base, mem_layout->total_size,
				MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_limit - ro_start,
				MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(coh_start, coh_limit - coh_start,
				MT_DEVICE | MT_RW | MT_SECURE);

	mmap_add(fvp_mmap);

	init_xlat_tables();

	enable_mmu();
	return;
}

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
