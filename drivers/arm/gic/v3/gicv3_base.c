/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>

#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gic.h>
#include <drivers/arm/gicv3.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#if USE_GIC_DRIVER != 3
#error "This file should only be used with USE_GIC_DRIVER=3"
#endif

#pragma weak gic_cpuif_enable
#pragma weak gic_cpuif_disable

/* The GICv3 driver only needs to be initialized in EL3 */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* List of zero terminated GICR frame addresses which CPUs will probe */
static const uintptr_t *gicr_frames = NULL;

static const interrupt_prop_t arm_interrupt_props[] = {
#ifdef PLAT_ARM_G1S_IRQ_PROPS
	PLAT_ARM_G1S_IRQ_PROPS(INTR_GROUP1S),
#endif
#ifdef PLAT_ARM_G0_IRQ_PROPS
	PLAT_ARM_G0_IRQ_PROPS(INTR_GROUP0),
#endif
#if ENABLE_FEAT_RAS && FFH_SUPPORT
	INTR_PROP_DESC(PLAT_CORE_FAULT_IRQ, PLAT_RAS_PRI, INTR_GROUP0,
			GIC_INTR_CFG_LEVEL)
#endif
};

/*
 * We save and restore the GICv3 context on system suspend. Allocate the
 * data in the designated EL3 Secure carve-out memory. The `used` attribute
 * is used to prevent the compiler from removing the gicv3 contexts.
 */
static gicv3_redist_ctx_t rdist_ctx __section(".arm_el3_tzc_dram") __used;
static gicv3_dist_ctx_t dist_ctx __section(".arm_el3_tzc_dram") __used;

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

gicv3_driver_data_t gic_data __unused = {
	.gicd_base = PLAT_ARM_GICD_BASE,
	/* unused for USE_GIC_DRIVER=3. Use gic_set_gicr_frames(), passing a ptr
	 * to an array with 2 values - the frame's base and a NULL pointer */
	.gicr_base = 0U,
	.interrupt_props = arm_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(arm_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = arm_gicv3_mpidr_hash
};

/*
 * Initialises the gicr_frames array. It contains a NULL terminated list of
 * non-contiguous blocks of GICR frames (located at uneven offsets). Most
 * platforms will have one such block, except multichip configurations, which
 * will usually have multiple.
 */
void gic_set_gicr_frames(const uintptr_t *plat_gicr_frames)
{
	assert(plat_gicr_frames != NULL);
	gicr_frames = plat_gicr_frames;
}

/******************************************************************************
 * ARM common helper to initialize the GIC. Only invoked by BL31
 *****************************************************************************/
void __init gic_init(unsigned int cpu_idx)
{
	gicv3_driver_init(&gic_data);
	gicv3_distif_init();
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void gic_cpuif_enable(unsigned int cpu_idx)
{
	gicv3_cpuif_enable(cpu_idx);
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void gic_cpuif_disable(unsigned int cpu_idx)
{
	gicv3_cpuif_disable(cpu_idx);
}

/******************************************************************************
 * ARM common helper function to iterate over all GICR frames and discover the
 * corresponding per-cpu redistributor frame as well as initialize the
 * corresponding interface in GICv3.
 *****************************************************************************/
void gic_pcpu_init(unsigned int cpu_idx)
{
	/* to guard against an empty array */
	int result = -1;
	const uintptr_t *frame = gicr_frames;

	/* did the platform initialise the array with gic_set_gicr_frames() */
	assert(gicr_frames != NULL);

	while (*frame != 0U) {
		result = gicv3_rdistif_probe(*frame);

		if (result == 0)
			break;

		frame++;
	}

	if (result == -1) {
		ERROR("No GICR base frame found for CPU 0x%lx\n", read_mpidr());
		panic();
	}
	gicv3_rdistif_init(cpu_idx);
}

/******************************************************************************
 * ARM common helpers to power GIC redistributor interface
 *****************************************************************************/
void gic_pcpu_off(unsigned int cpu_idx)
{
	gicv3_rdistif_off(cpu_idx);
}

/******************************************************************************
 * Common helper to save & restore the GICv3 on resume from system suspend. It
 * is the platform's responsibility to call these.
 *****************************************************************************/
void gic_save(void)
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

void gic_resume(void)
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
