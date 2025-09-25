/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
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

static const interrupt_prop_t __unused fvp_interrupt_props[] = {
	PLAT_ARM_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_ARM_G0_IRQ_PROPS(INTR_GROUP0)
};

extern gicv3_driver_data_t gic_data;

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

void fvp_pcpu_init(void)
{
	fvp_gicv3_make_rdistrif_rw();
}

void fvp_gic_driver_pre_init(void)
{
/* FCONF won't be used in these cases, so we couldn't do this */
#if !(RESET_TO_BL31 || RESET_TO_SP_MIN || RESET_TO_BL2)
	/*
	 * Get GICD and GICR base addressed through FCONF APIs.
	 * FCONF is not supported in BL32 for FVP.
	 */
#if (!defined(__aarch64__) && defined(IMAGE_BL32)) || \
	(defined(__aarch64__) && defined(IMAGE_BL31))
	gic_data.gicd_base = (uintptr_t)FCONF_GET_PROPERTY(hw_config,
							       gicv3_config,
							       gicd_base);
	arm_gicr_base_addrs[0] = FCONF_GET_PROPERTY(hw_config, gicv3_config,
						    gicr_base);
#if SEC_INT_DESC_IN_FCONF
	gic_data.interrupt_props = FCONF_GET_PROPERTY(hw_config,
					sec_intr_prop, descriptor);
	gic_data.interrupt_props_num = FCONF_GET_PROPERTY(hw_config,
					sec_intr_prop, count);
#else
	gic_data.interrupt_props = fvp_interrupt_props;
	gic_data.interrupt_props_num = ARRAY_SIZE(fvp_interrupt_props);
#endif
#else
	gic_data.gicd_base = PLAT_ARM_GICD_BASE;
	arm_gicr_base_addrs[0] = PLAT_ARM_GICR_BASE;
	gic_data.interrupt_props = fvp_interrupt_props;
	gic_data.interrupt_props_num = ARRAY_SIZE(fvp_interrupt_props);
#endif
#endif /* !(RESET_TO_BL31 || RESET_TO_SP_MIN || RESET_TO_BL2) */
	gic_set_gicr_frames(arm_gicr_base_addrs);
}
