/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#ifdef IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH1_RW,
	V2M_MAP_IOFPGA,
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH1_RW,
	V2M_MAP_IOFPGA,
	ARM_MAP_NS_DRAM1,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	{0}
};
#endif

ARM_CASSERT_MMAP

void __init fvp_ve_config_setup(void)
{
	unsigned int sys_id, arch;

	sys_id = mmio_read_32(V2M_SYSREGS_BASE + V2M_SYS_ID);
	arch = (sys_id >> V2M_SYS_ID_ARCH_SHIFT) & V2M_SYS_ID_ARCH_MASK;

	if (arch != ARCH_MODEL_VE) {
		ERROR("This firmware is for FVP VE models\n");
		panic();
	}
}

unsigned int plat_get_syscnt_freq2(void)
{
	return FVP_VE_TIMER_BASE_FREQUENCY;
}
