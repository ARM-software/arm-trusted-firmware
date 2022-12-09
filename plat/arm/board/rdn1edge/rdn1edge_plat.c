/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <sgi_soc_platform_def.h>
#include <sgi_plat.h>

#if defined(IMAGE_BL31)
static const mmap_region_t rdn1edge_dynamic_mmap[] = {
	ARM_MAP_SHARED_RAM_REMOTE_CHIP(1),
	CSS_SGI_MAP_DEVICE_REMOTE_CHIP(1),
	SOC_CSS_MAP_DEVICE_REMOTE_CHIP(1)
};

static struct gic600_multichip_data rdn1e1_multichip_data __init = {
	.rt_owner_base = PLAT_ARM_GICD_BASE,
	.rt_owner = 0,
	.chip_count = CSS_SGI_CHIP_COUNT,
	.chip_addrs = {
		PLAT_ARM_GICD_BASE >> 16,
		(PLAT_ARM_GICD_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(1)) >> 16
	},
	.spi_ids = {
		{32, 255},
		{0, 0}
	}
};

static uintptr_t rdn1e1_multichip_gicr_frames[] = {
	PLAT_ARM_GICR_BASE,				/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE +
		CSS_SGI_REMOTE_CHIP_MEM_OFFSET(1),	/* Chip 1's GICR BASE */
	UL(0)						/* Zero Termination */
};
#endif /* IMAGE_BL31 */

unsigned int plat_arm_sgi_get_platform_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_ID_OFFSET)
				& SID_SYSTEM_ID_PART_NUM_MASK;
}

unsigned int plat_arm_sgi_get_config_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_CFG_OFFSET);
}

unsigned int plat_arm_sgi_get_multi_chip_mode(void)
{
	return (mmio_read_32(SID_REG_BASE + SID_NODE_ID_OFFSET) &
			SID_MULTI_CHIP_MODE_MASK) >> SID_MULTI_CHIP_MODE_SHIFT;
}

/*
 * IMAGE_BL31 macro is added to build bl31_platform_setup function only for BL31
 * because PLAT_XLAT_TABLES_DYNAMIC macro is set to build only for BL31 and not
 * for other stages.
 */
#if defined(IMAGE_BL31)
void bl31_platform_setup(void)
{
	unsigned int i;
	int ret;

	if (plat_arm_sgi_get_multi_chip_mode() == 0 && CSS_SGI_CHIP_COUNT > 1) {
		ERROR("Chip Count is set to %d but multi-chip mode not enabled\n",
				CSS_SGI_CHIP_COUNT);
		panic();
	} else if (plat_arm_sgi_get_multi_chip_mode() == 1 &&
			CSS_SGI_CHIP_COUNT > 1) {
		INFO("Enabling support for multi-chip in RD-N1-Edge\n");

		for (i = 0; i < ARRAY_SIZE(rdn1edge_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdn1edge_dynamic_mmap[i].base_pa,
					rdn1edge_dynamic_mmap[i].base_va,
					rdn1edge_dynamic_mmap[i].size,
					rdn1edge_dynamic_mmap[i].attr
					);
			if (ret != 0) {
				ERROR("Failed to add dynamic mmap entry\n");
				panic();
			}
		}

		plat_arm_override_gicr_frames(rdn1e1_multichip_gicr_frames);
		gic600_multichip_init(&rdn1e1_multichip_data);
	}

	sgi_bl31_common_platform_setup();
}
#endif /* IMAGE_BL31 */
