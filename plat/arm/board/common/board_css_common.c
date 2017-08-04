/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arm_def.h>
#include <plat_arm.h>

/*
 * Table of memory regions for different BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as arm_setup_page_tables() already
 * takes care of mapping it.
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
#if TRUSTED_BOARD_BOOT
	/* Map DRAM to authenticate NS_BL2U image. */
	ARM_MAP_NS_DRAM1,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	ARM_MAP_NS_DRAM1,
	ARM_MAP_TSP_SEC_MEM,
	{0}
};
#endif
#ifdef IMAGE_BL2U
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#ifdef IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
#if CSS_USE_SCMI_DRIVER
	/*
	 * The SCMI payload area is currently in the Non Secure SRAM. This is
	 * a potential security risk but this will be resolved once SCP
	 * completely replaces SCPI with SCMI as the only communication
	 * protocol.
	 */
	CSS_MAP_NSRAM,
#endif
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
#ifdef AARCH32
	ARM_MAP_SHARED_RAM,
#endif
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif

ARM_CASSERT_MMAP

