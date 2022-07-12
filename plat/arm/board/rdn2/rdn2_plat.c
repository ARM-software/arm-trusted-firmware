/*
 * Copyright (c) 2020-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <sgi_soc_platform_def_v2.h>
#include <sgi_plat.h>

#if defined(IMAGE_BL31)
#if (CSS_SGI_PLATFORM_VARIANT == 2)
static const mmap_region_t rdn2mc_dynamic_mmap[] = {
#if CSS_SGI_CHIP_COUNT > 1
	ARM_MAP_SHARED_RAM_REMOTE_CHIP(1),
	CSS_SGI_MAP_DEVICE_REMOTE_CHIP(1),
#endif
#if CSS_SGI_CHIP_COUNT > 2
	ARM_MAP_SHARED_RAM_REMOTE_CHIP(2),
	CSS_SGI_MAP_DEVICE_REMOTE_CHIP(2),
#endif
#if CSS_SGI_CHIP_COUNT > 3
	ARM_MAP_SHARED_RAM_REMOTE_CHIP(3),
	CSS_SGI_MAP_DEVICE_REMOTE_CHIP(3),
#endif
};
#endif

#if (CSS_SGI_PLATFORM_VARIANT == 2)
static struct gic600_multichip_data rdn2mc_multichip_data __init = {
	.rt_owner_base = PLAT_ARM_GICD_BASE,
	.rt_owner = 0,
	.chip_count = CSS_SGI_CHIP_COUNT,
	.chip_addrs = {
		PLAT_ARM_GICD_BASE >> 16,
#if CSS_SGI_CHIP_COUNT > 1
		(PLAT_ARM_GICD_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
#endif
#if CSS_SGI_CHIP_COUNT > 2
		(PLAT_ARM_GICD_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
#endif
#if CSS_SGI_CHIP_COUNT > 3
		(PLAT_ARM_GICD_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
#endif
	},
	.spi_ids = {
		{32, 511},
	#if CSS_SGI_CHIP_COUNT > 1
		{512, 991},
	#endif
	#if CSS_SGI_CHIP_COUNT > 2
		{4096, 4575},
	#endif
	#if CSS_SGI_CHIP_COUNT > 3
		{4576, 5055},
	#endif
	}
};
#endif

#if (CSS_SGI_PLATFORM_VARIANT == 2)
static uintptr_t rdn2mc_multichip_gicr_frames[] = {
	/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE,
#if CSS_SGI_CHIP_COUNT > 1
	/* Chip 1's GICR BASE */
	PLAT_ARM_GICR_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(1),
#endif
#if CSS_SGI_CHIP_COUNT > 2
	/* Chip 2's GICR BASE */
	PLAT_ARM_GICR_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(2),
#endif
#if CSS_SGI_CHIP_COUNT > 3
	/* Chip 3's GICR BASE */
	PLAT_ARM_GICR_BASE + CSS_SGI_REMOTE_CHIP_MEM_OFFSET(3),
#endif
	UL(0)	/* Zero Termination */
};
#endif
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
			     SID_MULTI_CHIP_MODE_MASK) >>
			     SID_MULTI_CHIP_MODE_SHIFT;
}

#if defined(IMAGE_BL31)
void bl31_platform_setup(void)
{
#if (CSS_SGI_PLATFORM_VARIANT == 2)
	int ret;
	unsigned int i;

	if (plat_arm_sgi_get_multi_chip_mode() == 0) {
		ERROR("Chip Count is set to %u but multi-chip mode is not "
			"enabled\n", CSS_SGI_CHIP_COUNT);
		panic();
	} else {
		INFO("Enabling multi-chip support for RD-N2 variant\n");

		for (i = 0; i < ARRAY_SIZE(rdn2mc_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdn2mc_dynamic_mmap[i].base_pa,
					rdn2mc_dynamic_mmap[i].base_va,
					rdn2mc_dynamic_mmap[i].size,
					rdn2mc_dynamic_mmap[i].attr);
			if (ret != 0) {
				ERROR("Failed to add dynamic mmap entry for"
					" i: %d " "(ret=%d)\n", i, ret);
				panic();
			}
		}

		plat_arm_override_gicr_frames(
			rdn2mc_multichip_gicr_frames);
		gic600_multichip_init(&rdn2mc_multichip_data);
	}
#endif

	sgi_bl31_common_platform_setup();
}
#endif /* IMAGE_BL31 */
