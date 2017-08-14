/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_def.h>
#include <gicv3.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv3 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_gic_driver_init
#pragma weak plat_arm_gic_init
#pragma weak plat_arm_gic_cpuif_enable
#pragma weak plat_arm_gic_cpuif_disable
#pragma weak plat_arm_gic_pcpu_init
#pragma weak plat_arm_gic_redistif_on
#pragma weak plat_arm_gic_redistif_off

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* Array of Group1 secure interrupts to be configured by the gic driver */
static const unsigned int g1s_interrupt_array[] = {
	PLAT_ARM_G1S_IRQS
};

/* Array of Group0 interrupts to be configured by the gic driver */
static const unsigned int g0_interrupt_array[] = {
	PLAT_ARM_G0_IRQS
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
static unsigned int arm_gicv3_mpidr_hash(u_register_t mpidr)
{
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return plat_arm_calc_core_pos(mpidr);
}

const gicv3_driver_data_t arm_gic_data = {
	.gicd_base = PLAT_ARM_GICD_BASE,
	.gicr_base = PLAT_ARM_GICR_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g1s_interrupt_num = ARRAY_SIZE(g1s_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
	.g1s_interrupt_array = g1s_interrupt_array,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = arm_gicv3_mpidr_hash
};

void plat_arm_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if (defined(AARCH32) && defined(IMAGE_BL32)) || \
	(defined(IMAGE_BL31) && !defined(AARCH32))
	gicv3_driver_init(&arm_gic_data);
#endif
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void plat_arm_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helper to initialize the per-cpu redistributor interface in GICv3
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

/******************************************************************************
 * ARM common helpers to power GIC redistributor interface
 *****************************************************************************/
void plat_arm_gic_redistif_on(void)
{
	gicv3_rdistif_on(plat_my_core_pos());
}

void plat_arm_gic_redistif_off(void)
{
	gicv3_rdistif_off(plat_my_core_pos());
}
