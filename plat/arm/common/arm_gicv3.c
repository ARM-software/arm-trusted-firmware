/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>

#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

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

/* Default GICR base address to be used for GICR probe. */
static const uintptr_t gicr_base_addrs[2] = {
	PLAT_ARM_GICR_BASE,	/* GICR Base address of the primary CPU */
	0U			/* Zero Termination */
};

/* List of zero terminated GICR frame addresses which CPUs will probe */
static const uintptr_t *gicr_frames = gicr_base_addrs;

static const interrupt_prop_t arm_interrupt_props[] = {
	PLAT_ARM_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_ARM_G0_IRQ_PROPS(INTR_GROUP0)
};

/*
 * We save and restore the GICv3 context on system suspend. Allocate the
 * data in the designated EL3 Secure carve-out memory. The `used` attribute
 * is used to prevent the compiler from removing the gicv3 contexts.
 */
static gicv3_redist_ctx_t rdist_ctx __section("arm_el3_tzc_dram") __used;
static gicv3_dist_ctx_t dist_ctx __section("arm_el3_tzc_dram") __used;

/* Define accessor function to get reference to the GICv3 context */
DEFINE_LOAD_SYM_ADDR(rdist_ctx)
DEFINE_LOAD_SYM_ADDR(dist_ctx)

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

static const gicv3_driver_data_t arm_gic_data __unused = {
	.gicd_base = PLAT_ARM_GICD_BASE,
	.gicr_base = 0U,
	.interrupt_props = arm_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(arm_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = arm_gicv3_mpidr_hash
};

/*
 * By default, gicr_frames will be pointing to gicr_base_addrs. If
 * the platform supports a non-contiguous GICR frames (GICR frames located
 * at uneven offset), plat_arm_override_gicr_frames function can be used by
 * such platform to override the gicr_frames.
 */
void plat_arm_override_gicr_frames(const uintptr_t *plat_gicr_frames)
{
	assert(plat_gicr_frames != NULL);
	gicr_frames = plat_gicr_frames;
}

void __init plat_arm_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if (!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))
	gicv3_driver_init(&arm_gic_data);

	if (gicv3_rdistif_probe(gicr_base_addrs[0]) == -1) {
		ERROR("No GICR base frame found for Primary CPU\n");
		panic();
	}
#endif
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void __init plat_arm_gic_init(void)
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
 * ARM common helper function to iterate over all GICR frames and discover the
 * corresponding per-cpu redistributor frame as well as initialize the
 * corresponding interface in GICv3.
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	int result;
	const uintptr_t *plat_gicr_frames = gicr_frames;

	do {
		result = gicv3_rdistif_probe(*plat_gicr_frames);

		/* If the probe is successful, no need to proceed further */
		if (result == 0)
			break;

		plat_gicr_frames++;
	} while (*plat_gicr_frames != 0U);

	if (result == -1) {
		ERROR("No GICR base frame found for CPU 0x%lx\n", read_mpidr());
		panic();
	}
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

/******************************************************************************
 * ARM common helper to save & restore the GICv3 on resume from system suspend
 *****************************************************************************/
void plat_arm_gic_save(void)
{
	gicv3_redist_ctx_t * const rdist_context =
			(gicv3_redist_ctx_t *)LOAD_ADDR_OF(rdist_ctx);
	gicv3_dist_ctx_t * const dist_context =
			(gicv3_dist_ctx_t *)LOAD_ADDR_OF(dist_ctx);

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
	gicv3_rdistif_save(plat_my_core_pos(), rdist_context);

	/* Save the GIC Distributor context */
	gicv3_distif_save(dist_context);

	/*
	 * From here, all the components of the GIC can be safely powered down
	 * as long as there is an alternate way to handle wakeup interrupt
	 * sources.
	 */
}

void plat_arm_gic_resume(void)
{
	const gicv3_redist_ctx_t *rdist_context =
			(gicv3_redist_ctx_t *)LOAD_ADDR_OF(rdist_ctx);
	const gicv3_dist_ctx_t *dist_context =
			(gicv3_dist_ctx_t *)LOAD_ADDR_OF(dist_ctx);

	/* Restore the GIC Distributor context */
	gicv3_distif_init_restore(dist_context);

	/*
	 * Restore the GIC Redistributor and ITS contexts after the
	 * Distributor context. As we only handle SYSTEM SUSPEND API,
	 * we only need to restore the context of the CPU that issued
	 * the SYSTEM SUSPEND call.
	 */
	gicv3_rdistif_init_restore(plat_my_core_pos(), rdist_context);

	/*
	 * If an ITS is available, restore its context after
	 * the Redistributor using:
	 * gicv3_its_restore(gits_base, &its_ctx[i])
	 * An implementation-defined sequence may be required to
	 * restore the whole ITS state. The ITS must also be
	 * re-enabled after this sequence has been executed.
	 */
}
