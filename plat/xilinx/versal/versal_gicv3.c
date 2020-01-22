/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_private.h>
#include <platform_def.h>

#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv3 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_versal_gic_driver_init
#pragma weak plat_versal_gic_init
#pragma weak plat_versal_gic_cpuif_enable
#pragma weak plat_versal_gic_cpuif_disable
#pragma weak plat_versal_gic_pcpu_init
#pragma weak plat_versal_gic_redistif_on
#pragma weak plat_versal_gic_redistif_off

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t versal_interrupt_props[] = {
	PLAT_VERSAL_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_VERSAL_G0_IRQ_PROPS(INTR_GROUP0)
};

/*
 * We save and restore the GICv3 context on system suspend. Allocate the
 * data in the designated EL3 Secure carve-out memory.
 */
static gicv3_redist_ctx_t rdist_ctx __section("versal_el3_tzc_dram");
static gicv3_dist_ctx_t dist_ctx __section("versal_el3_tzc_dram");

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
static unsigned int versal_gicv3_mpidr_hash(u_register_t mpidr)
{
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return versal_calc_core_pos(mpidr);
}

static const gicv3_driver_data_t versal_gic_data __unused = {
	.gicd_base = PLAT_VERSAL_GICD_BASE,
	.gicr_base = PLAT_VERSAL_GICR_BASE,
	.interrupt_props = versal_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(versal_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = versal_gicv3_mpidr_hash
};

void __init plat_versal_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if IMAGE_BL31
	gicv3_driver_init(&versal_gic_data);
#endif
}

/******************************************************************************
 * Versal common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void __init plat_versal_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * Versal common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_versal_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * Versal common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_versal_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * Versal common helper to initialize the per-cpu redistributor interface in
 * GICv3
 *****************************************************************************/
void plat_versal_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

/******************************************************************************
 * Versal common helpers to power GIC redistributor interface
 *****************************************************************************/
void plat_versal_gic_redistif_on(void)
{
	gicv3_rdistif_on(plat_my_core_pos());
}

void plat_versal_gic_redistif_off(void)
{
	gicv3_rdistif_off(plat_my_core_pos());
}

/******************************************************************************
 * Versal common helper to save & restore the GICv3 on resume from system
 * suspend
 *****************************************************************************/
void plat_versal_gic_save(void)
{
	/*
	 * If an ITS is available, save its context before
	 * the Redistributor using:
	 * gicv3_its_save_disable(gits_base, &its_ctx[i])
	 * Additionnaly, an implementation-defined sequence may
	 * be required to save the whole ITS state.
	 */

	/*
	 * Save the GIC Redistributors and ITS contexts before the
	 * Distributor context. As we only handle SYSTEM SUSPEND API,
	 * we only need to save the context of the CPU that is issuing
	 * the SYSTEM SUSPEND call, i.e. the current CPU.
	 */
	gicv3_rdistif_save(plat_my_core_pos(), &rdist_ctx);

	/* Save the GIC Distributor context */
	gicv3_distif_save(&dist_ctx);

	/*
	 * From here, all the components of the GIC can be safely powered down
	 * as long as there is an alternate way to handle wakeup interrupt
	 * sources.
	 */
}

void plat_versal_gic_resume(void)
{
	/* Restore the GIC Distributor context */
	gicv3_distif_init_restore(&dist_ctx);

	/*
	 * Restore the GIC Redistributor and ITS contexts after the
	 * Distributor context. As we only handle SYSTEM SUSPEND API,
	 * we only need to restore the context of the CPU that issued
	 * the SYSTEM SUSPEND call.
	 */
	gicv3_rdistif_init_restore(plat_my_core_pos(), &rdist_ctx);

	/*
	 * If an ITS is available, restore its context after
	 * the Redistributor using:
	 * gicv3_its_restore(gits_base, &its_ctx[i])
	 * An implementation-defined sequence may be required to
	 * restore the whole ITS state. The ITS must also be
	 * re-enabled after this sequence has been executed.
	 */
}
