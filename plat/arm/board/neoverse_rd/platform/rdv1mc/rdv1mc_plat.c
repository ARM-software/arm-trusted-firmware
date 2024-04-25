/*
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gic600_multichip.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <nrd_plat.h>

#if defined(IMAGE_BL31)
static const mmap_region_t rdv1mc_dynamic_mmap[] = {
	NRD_CSS_SHARED_RAM_MMAP(1),
	NRD_CSS_PERIPH_MMAP(1),
	NRD_ROS_PERIPH_MMAP(1),
#if (NRD_CHIP_COUNT > 2)
	NRD_CSS_SHARED_RAM_MMAP(2),
	NRD_CSS_PERIPH_MMAP(2),
	NRD_ROS_PERIPH_MMAP(2),
#endif
#if (NRD_CHIP_COUNT > 3)
	NRD_CSS_SHARED_RAM_MMAP(3),
	NRD_CSS_PERIPH_MMAP(3),
	NRD_ROS_PERIPH_MMAP(3)
#endif
};

static struct gic600_multichip_data rdv1mc_multichip_data __init = {
	.rt_owner_base = PLAT_ARM_GICD_BASE,
	.rt_owner = 0,
	.chip_count = NRD_CHIP_COUNT,
	.chip_addrs = {
		PLAT_ARM_GICD_BASE >> 16,
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1)) >> 16,
#if (NRD_CHIP_COUNT > 2)
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2)) >> 16,
#endif
#if (NRD_CHIP_COUNT > 3)
		(PLAT_ARM_GICD_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3)) >> 16,
#endif
	},
	.spi_ids = {
		{PLAT_ARM_GICD_BASE,
		NRD_CHIP0_SPI_MIN,
		NRD_CHIP0_SPI_MAX},
		{0, 0, 0},
#if (NRD_CHIP_COUNT > 2)
		{0, 0, 0},
#endif
#if (NRD_CHIP_COUNT > 3)
		{0, 0, 0},
#endif
	}
};

static uintptr_t rdv1mc_multichip_gicr_frames[] = {
	/* Chip 0's GICR Base */
	PLAT_ARM_GICR_BASE,
	/* Chip 1's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(1),
#if (NRD_CHIP_COUNT > 2)
	/* Chip 2's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(2),
#endif
#if (NRD_CHIP_COUNT > 3)
	/* Chip 3's GICR BASE */
	PLAT_ARM_GICR_BASE + NRD_REMOTE_CHIP_MEM_OFFSET(3),
#endif
	UL(0)	/* Zero Termination */
};
#endif /* IMAGE_BL31 */

unsigned int plat_arm_nrd_get_platform_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_ID_OFFSET)
				& SID_SYSTEM_ID_PART_NUM_MASK;
}

unsigned int plat_arm_nrd_get_config_id(void)
{
	return mmio_read_32(SID_REG_BASE + SID_SYSTEM_CFG_OFFSET);
}

unsigned int plat_arm_nrd_get_multi_chip_mode(void)
{
	return (mmio_read_32(SID_REG_BASE + SID_NODE_ID_OFFSET) &
			SID_MULTI_CHIP_MODE_MASK) >> SID_MULTI_CHIP_MODE_SHIFT;
}

/*
 * bl31_platform_setup_function is guarded by IMAGE_BL31 macro because
 * PLAT_XLAT_TABLES_DYNAMIC macro is set to build only for BL31 and not
 * for other stages.
 */
#if defined(IMAGE_BL31)
void bl31_platform_setup(void)
{
	int ret;
	unsigned int i;

	if ((plat_arm_nrd_get_multi_chip_mode() == 0) &&
			(NRD_CHIP_COUNT > 1)) {
		ERROR("Chip Count is %u but multi-chip mode is not enabled\n",
			NRD_CHIP_COUNT);
		panic();
	} else if ((plat_arm_nrd_get_multi_chip_mode() == 1) &&
			(NRD_CHIP_COUNT > 1)) {
		INFO("Enabling support for multi-chip in RD-V1-MC\n");

		for (i = 0; i < ARRAY_SIZE(rdv1mc_dynamic_mmap); i++) {
			ret = mmap_add_dynamic_region(
					rdv1mc_dynamic_mmap[i].base_pa,
					rdv1mc_dynamic_mmap[i].base_va,
					rdv1mc_dynamic_mmap[i].size,
					rdv1mc_dynamic_mmap[i].attr);
			if (ret != 0) {
				ERROR("Failed to add dynamic mmap entry "
						"(ret=%d)\n", ret);
				panic();
			}
		}

		plat_arm_override_gicr_frames(
			rdv1mc_multichip_gicr_frames);
		gic600_multichip_init(&rdv1mc_multichip_data);
	}

	nrd_bl31_common_platform_setup();
}
#endif /* IMAGE_BL31 */
