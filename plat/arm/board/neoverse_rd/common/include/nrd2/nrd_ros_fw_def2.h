/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the RoS firmware specific definitions for the
 * second generation platforms based on the N2/V2 CPUs. RoS (Rest Of System) is
 * used to refer to the part of the reference design platform that excludes CSS.
 */

#ifndef NRD_ROS_FW_DEF2_H
#define NRD_ROS_FW_DEF2_H

#include <nrd_ros_def2.h>

/*******************************************************************************
 * MMU mapping
 ******************************************************************************/

#define SOC_MEMCNTRL_MAP_DEVICE						\
		MAP_REGION_FLAT(					\
			SOC_MEMCNTRL_BASE,				\
			SOC_MEMCNTRL_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

#define SOC_MEMCNTRL_MAP_DEVICE_REMOTE_CHIP(n)				\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			SOC_MEMCNTRL_BASE,				\
			SOC_MEMCNTRL_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

#define PLAT_ARM_SECURE_MAP_SYSTEMREG					\
		MAP_REGION_FLAT(					\
			CSS_SYSTEMREG_DEVICE_BASE,			\
			CSS_SYSTEMREG_DEVICE_SIZE,			\
			MT_DEVICE | MT_RW | MT_SECURE | MT_USER)

#define PLAT_ARM_SECURE_MAP_NOR2					\
		MAP_REGION_FLAT(					\
			CSS_NOR2_FLASH_DEVICE_BASE,			\
			CSS_NOR2_FLASH_DEVICE_SIZE,			\
			MT_DEVICE | MT_RW | MT_SECURE | MT_USER)

/*******************************************************************************
 * TZ config
 ******************************************************************************/

/*
 * Mapping definition of the TrustZone Controller for Arm Neoverse RD platforms
 * where both the DRAM regions are marked for non-secure access. This applies
 * to multi-chip platforms.
 */
#define NRD_PLAT_TZC_NS_REMOTE_REGIONS_DEF(n)				\
	{NRD_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM1_BASE,		\
		NRD_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM1_END,		\
		ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS},	\
	{NRD_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM2_BASE,		\
		NRD_REMOTE_CHIP_MEM_OFFSET(n) + ARM_DRAM2_END,		\
		ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS}

#endif /* NRD_ROS_FW_DEF2_H */
