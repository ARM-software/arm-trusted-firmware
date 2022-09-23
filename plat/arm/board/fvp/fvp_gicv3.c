/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>

#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <fconf_hw_config_getter.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/fconf_sec_intr_config.h>
#include <plat/common/platform.h>

#if FVP_GICR_REGION_PROTECTION
/* To indicate GICR region of the core initialized as Read-Write */
static bool fvp_gicr_rw_region_init[PLATFORM_CORE_COUNT] = {false};
#endif /* FVP_GICR_REGION_PROTECTION */

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t fvp_rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* Default GICR base address to be used for GICR probe. */
static uint64_t fvp_gicr_base_addrs[2] = { 0U };

/* List of zero terminated GICR frame addresses which CPUs will probe */
static uint64_t *fvp_gicr_frames = fvp_gicr_base_addrs;

#if  !(SEC_INT_DESC_IN_FCONF && ((!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))))
static const interrupt_prop_t fvp_interrupt_props[] = {
	PLAT_ARM_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_ARM_G0_IRQ_PROPS(INTR_GROUP0)
};
#endif

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
static unsigned int fvp_gicv3_mpidr_hash(u_register_t mpidr)
{
	u_register_t temp_mpidr = mpidr;

	temp_mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return plat_arm_calc_core_pos(temp_mpidr);
}


static gicv3_driver_data_t fvp_gic_data = {
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = fvp_rdistif_base_addrs,
	.mpidr_to_core_pos = fvp_gicv3_mpidr_hash
};

/******************************************************************************
 * This function gets called per core to make its redistributor frame rw
 *****************************************************************************/
static void fvp_gicv3_make_rdistrif_rw(void)
{
#if FVP_GICR_REGION_PROTECTION
	unsigned int core_pos = plat_my_core_pos();

	/* Make the redistributor frame RW if it is not done previously */
	if (fvp_gicr_rw_region_init[core_pos] != true) {
		int ret = xlat_change_mem_attributes(BASE_GICR_BASE +
						     (core_pos * BASE_GICR_SIZE),
						     BASE_GICR_SIZE,
						     MT_EXECUTE_NEVER |
						     MT_DEVICE | MT_RW |
						     MT_SECURE);

		if (ret != 0) {
			ERROR("Failed to make redistributor frame \
			       read write = %d\n", ret);
			panic();
		} else {
			fvp_gicr_rw_region_init[core_pos] = true;
		}
	}
#else
	return;
#endif /* FVP_GICR_REGION_PROTECTION */
}

void plat_arm_gic_driver_init(void)
{
	fvp_gicv3_make_rdistrif_rw();
	/*
	 * Get GICD and GICR base addressed through FCONF APIs.
	 * FCONF is not supported in BL32 for FVP.
	 */
#if (!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))
	fvp_gic_data.gicd_base = (uintptr_t)FCONF_GET_PROPERTY(hw_config,
							       gicv3_config,
							       gicd_base);
	fvp_gicr_base_addrs[0] = FCONF_GET_PROPERTY(hw_config, gicv3_config,
						    gicr_base);
#if SEC_INT_DESC_IN_FCONF
	fvp_gic_data.interrupt_props = FCONF_GET_PROPERTY(hw_config,
					sec_intr_prop, descriptor);
	fvp_gic_data.interrupt_props_num = FCONF_GET_PROPERTY(hw_config,
					sec_intr_prop, count);
#else
	fvp_gic_data.interrupt_props = fvp_interrupt_props;
	fvp_gic_data.interrupt_props_num = ARRAY_SIZE(fvp_interrupt_props);
#endif
#else
	fvp_gic_data.gicd_base = PLAT_ARM_GICD_BASE;
	fvp_gicr_base_addrs[0] = PLAT_ARM_GICR_BASE;
	fvp_gic_data.interrupt_props = fvp_interrupt_props;
	fvp_gic_data.interrupt_props_num = ARRAY_SIZE(fvp_interrupt_props);
#endif

	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */

#if (!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))
	gicv3_driver_init(&fvp_gic_data);
	if (gicv3_rdistif_probe((uintptr_t)fvp_gicr_base_addrs[0]) == -1) {
		ERROR("No GICR base frame found for Primary CPU\n");
		panic();
	}
#endif
}

/******************************************************************************
 * Function to iterate over all GICR frames and discover the corresponding
 * per-cpu redistributor frame as well as initialize the corresponding
 * interface in GICv3.
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	int result;
	const uint64_t *plat_gicr_frames = fvp_gicr_frames;

	fvp_gicv3_make_rdistrif_rw();

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
