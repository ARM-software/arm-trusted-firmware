/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>

#include <sgm_variant.h>

/*
 * Table of regions for different BL stages to map using the MMU.
 * This doesn't include Trusted RAM as the 'mem_layout' argument passed to
 * arm_configure_mmu_elx() will give the available subset of that.
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	CSS_MAP_GIC_DEVICE,
	SOC_CSS_MAP_DEVICE,
#if TRUSTED_BOARD_BOOT
	ARM_MAP_NS_DRAM1,
#endif
	{0}
};
#endif
#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	CSS_MAP_GIC_DEVICE,
	SOC_CSS_MAP_DEVICE,
	ARM_MAP_NS_DRAM1,
#ifdef SPD_tspd
	ARM_MAP_TSP_SEC_MEM,
#endif
#ifdef SPD_opteed
	ARM_OPTEE_PAGEABLE_LOAD_MEM,
#endif
#if TRUSTED_BOARD_BOOT && !BL2_AT_EL3
	ARM_MAP_BL1_RW,
#endif
	{0}
};
#endif
#if IMAGE_BL2U
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	CSS_MAP_DEVICE,
	CSS_MAP_GIC_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	CSS_MAP_GIC_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#if IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	CSS_MAP_GIC_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif

ARM_CASSERT_MMAP

const mmap_region_t *plat_arm_get_mmap(void)
{
	return plat_arm_mmap;
}
