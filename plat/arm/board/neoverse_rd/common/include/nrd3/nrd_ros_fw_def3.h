/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the RoS firmware specific definitions for the
 * third generation of platforms. RoS (Rest Of System) is used to refer to the
 * part of the reference design platform that excludes CSS.
 */

#ifndef NRD_ROS_FW_DEF3_H
#define NRD_ROS_FW_DEF3_H

#include <nrd_ros_def3.h>

/*******************************************************************************
 * MMU mapping
 ******************************************************************************/

#define NRD_ROS_PLATFORM_PERIPH_MMAP					\
		MAP_REGION_FLAT(					\
			NRD_ROS_PLATFORM_PERIPH_BASE,			\
			NRD_ROS_PLATFORM_PERIPH_SIZE,			\
			MT_DEVICE | MT_RW | MT_SECURE)

#define NRD_ROS_SYSTEM_PERIPH_MMAP					\
		MAP_REGION_FLAT(					\
			NRD_ROS_SYSTEM_PERIPH_BASE,			\
			NRD_ROS_SYSTEM_PERIPH_SIZE,			\
			MT_DEVICE | MT_RW | MT_SECURE)

#define NRD_ROS_V2M_MEM_PROTECT_MMAP					\
		MAP_REGION_FLAT(					\
			PLAT_ARM_MEM_PROT_ADDR,				\
			V2M_FLASH_BLOCK_SIZE,				\
			MT_DEVICE | MT_RW | EL3_PAS)

#define NRD_ROS_FLASH0_RO_MMAP						\
		MAP_REGION_FLAT(					\
			V2M_FLASH0_BASE,				\
			V2M_FLASH0_SIZE,				\
			MT_DEVICE | MT_RO | MT_SECURE)

#endif /* NRD_ROS_FW_DEF3_H */
