/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <plat/common/platform.h>

#include <marvell_def.h>
#include <plat_marvell.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv3 driver is initialised and used.
 ******************************************************************************
 */
#pragma weak plat_marvell_gic_driver_init
#pragma weak plat_marvell_gic_init
#pragma weak plat_marvell_gic_cpuif_enable
#pragma weak plat_marvell_gic_cpuif_disable
#pragma weak plat_marvell_gic_pcpu_init

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t marvell_interrupt_props[] = {
	PLAT_MARVELL_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_MARVELL_G0_IRQ_PROPS(INTR_GROUP0)
};

/*
 * We save and restore the GICv3 context on system suspend. Allocate the
 * data in the designated EL3 Secure carve-out memory
 */
static gicv3_redist_ctx_t rdist_ctx __section("arm_el3_tzc_dram");
static gicv3_dist_ctx_t dist_ctx __section("arm_el3_tzc_dram");

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
static unsigned int marvell_gicv3_mpidr_hash(u_register_t mpidr)
{
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return plat_marvell_calc_core_pos(mpidr);
}

const gicv3_driver_data_t marvell_gic_data = {
	.gicd_base = PLAT_MARVELL_GICD_BASE,
	.gicr_base = PLAT_MARVELL_GICR_BASE,
	.interrupt_props = marvell_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(marvell_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = marvell_gicv3_mpidr_hash
};

void plat_marvell_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if IMAGE_BL31
	gicv3_driver_init(&marvell_gic_data);
#endif
}

/******************************************************************************
 * Marvell common helper to initialize the GIC. Only invoked by BL31
 ******************************************************************************
 */
void plat_marvell_gic_init(void)
{
	/* Initialize GIC-600 Multi Chip feature,
	 * only if the maximum number of north bridges
	 * is more than 1 - otherwise no need for multi
	 * chip feature initialization
	 */
#if (PLAT_MARVELL_NORTHB_COUNT > 1)
	if (gic600_multi_chip_init())
		ERROR("GIC-600 Multi Chip initialization failed\n");
#endif
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * Marvell common helper to enable the GIC CPU interface
 ******************************************************************************
 */
void plat_marvell_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * Marvell common helper to disable the GIC CPU interface
 ******************************************************************************
 */
void plat_marvell_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * Marvell common helper to init. the per-cpu redistributor interface in GICv3
 ******************************************************************************
 */
void plat_marvell_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

/******************************************************************************
 * Marvell common helper to save SPI irq states in GICv3
 ******************************************************************************
 */
void plat_marvell_gic_irq_save(void)
{

	/*
	 * If an ITS is available, save its context before
	 * the Redistributor using:
	 * gicv3_its_save_disable(gits_base, &its_ctx[i])
	 * Additionally, an implementation-defined sequence may
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

/******************************************************************************
 * Marvell common helper to restore SPI irq states in GICv3
 ******************************************************************************
 */
void plat_marvell_gic_irq_restore(void)
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

/******************************************************************************
 * Marvell common helper to save per-cpu PPI irq states in GICv3
 ******************************************************************************
 */
void plat_marvell_gic_irq_pcpu_save(void)
{
	gicv3_rdistif_save(plat_my_core_pos(), &rdist_ctx);
}

/******************************************************************************
 * Marvell common helper to restore per-cpu PPI irq states in GICv3
 ******************************************************************************
 */
void plat_marvell_gic_irq_pcpu_restore(void)
{
	gicv3_rdistif_init_restore(plat_my_core_pos(), &rdist_ctx);
}
