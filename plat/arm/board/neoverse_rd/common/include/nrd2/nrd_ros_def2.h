/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file contains the RoS specific definitions for the second generation
 * platforms based on the N2/V2 CPU.
 */

#ifndef NRD_ROS_DEF2_H
#define NRD_ROS_DEF2_H

/*******************************************************************************
 * SoC memory map related defines
 ******************************************************************************/

/* System Reg */
#define NRD_ROS_SYSTEMREG_BASE			UL(0x0C010000)
#define NRD_ROS_SYSTEMREG_SIZE			UL(0x00010000)

/* NOR flash 2 */
#define NRD_ROS_NOR2_FLASH_BASE			ULL(0x001054000000)
#define NRD_ROS_NOR2_FLASH_SIZE			UL(0x000004000000)

/* Memory controller */
#define NRD_ROS_MEMCNTRL_BASE			UL(0x10000000)
#define NRD_ROS_MEMCNTRL_SIZE			UL(0x10000000)

/* System peripherals */
#define NRD_ROS_SYSTEM_PERIPH_BASE		UL(0x0C000000)
#define NRD_ROS_SYSTEM_PERIPH_SIZE		UL(0x02000000)

/* Platform peripherals */
#define NRD_ROS_PLATFORM_PERIPH_BASE		UL(0x0E000000)
#define NRD_ROS_PLATFORM_PERIPH_SIZE		UL(0x02000000)

/* SMC0 */
#define NRD_ROS_SMC0_BASE			UL(0x08000000)
#define NRD_ROS_SMC0_SIZE			UL(0x04000000)

#endif /* NRD_ROS_DEF2_H */
