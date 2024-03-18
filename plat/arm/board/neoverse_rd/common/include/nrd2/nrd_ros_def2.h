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
#define CSS_SYSTEMREG_DEVICE_BASE		UL(0x0C010000)
#define CSS_SYSTEMREG_DEVICE_SIZE		UL(0x00010000)

/* NOR flash 2 */
#define CSS_NOR2_FLASH_DEVICE_BASE		ULL(0x001054000000)
#define CSS_NOR2_FLASH_DEVICE_SIZE		UL(0x000004000000)

/* Memory controller */
#define SOC_MEMCNTRL_BASE			UL(0x10000000)
#define SOC_MEMCNTRL_SIZE			UL(0x10000000)

/* System peripherals */
#define SOC_SYSTEM_PERIPH_BASE			UL(0x0C000000)
#define SOC_SYSTEM_PERIPH_SIZE			UL(0x02000000)

/* Platform peripherals */
#define SOC_PLATFORM_PERIPH_BASE		UL(0x0E000000)
#define SOC_PLATFORM_PERIPH_SIZE		UL(0x02000000)

#endif /* NRD_ROS_DEF2_H */
