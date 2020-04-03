/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gicv3.h>
#include <plat/common/platform.h>

#include <platform_def.h>

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t brcm_rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t brcm_interrupt_props[] = {
	/* G1S interrupts */
	PLAT_BRCM_G1S_IRQ_PROPS(INTR_GROUP1S),
	/* G0 interrupts */
	PLAT_BRCM_G0_IRQ_PROPS(INTR_GROUP0)
};

/*
 * MPIDR hashing function for translating MPIDRs read from GICR_TYPER register
 * to core position.
 *
 * Calculating core position is dependent on MPIDR_EL1.MT bit. However, affinity
 * values read from GICR_TYPER don't have an MT field. To reuse the same
 * translation used for CPUs, we insert MT bit read from the PE's MPIDR into
 * that read from GICR_TYPER.
 *
 * Assumptions:
 *
 *   - All CPUs implemented in the system have MPIDR_EL1.MT bit set;
 *   - No CPUs implemented in the system use affinity level 3.
 */
static unsigned int brcm_gicv3_mpidr_hash(u_register_t mpidr)
{
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return plat_core_pos_by_mpidr(mpidr);
}

static const gicv3_driver_data_t brcm_gic_data = {
	.gicd_base = PLAT_BRCM_GICD_BASE,
	.gicr_base = PLAT_BRCM_GICR_BASE,
	.interrupt_props = brcm_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(brcm_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = brcm_rdistif_base_addrs,
	.mpidr_to_core_pos = brcm_gicv3_mpidr_hash
};

void plat_brcm_gic_driver_init(void)
{
	/* TODO Check if this is required to be initialized here
	 * after getting initialized in EL3, should we re-init this here
	 * in S-EL1
	 */
	gicv3_driver_init(&brcm_gic_data);
}

void plat_brcm_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void plat_brcm_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void plat_brcm_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void plat_brcm_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

void plat_brcm_gic_redistif_on(void)
{
	gicv3_rdistif_on(plat_my_core_pos());
}

void plat_brcm_gic_redistif_off(void)
{
	gicv3_rdistif_off(plat_my_core_pos());
}
