/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file contains the RoS specific definitions for the third generation of
 * platforms.
 */

#ifndef NRD_ROS_DEF3_H
#define NRD_ROS_DEF3_H

/*******************************************************************************
 * RoS memory map related defines
 ******************************************************************************/

/* System peripherals */
#define NRD_ROS_SYSTEM_PERIPH_BASE		UL(0x0C000000)
#define NRD_ROS_SYSTEM_PERIPH_SIZE		UL(0x02000000)

/* Platform peripherals */
#define NRD_ROS_PLATFORM_PERIPH_BASE		UL(0x0E000000)
#define NRD_ROS_PLATFORM_PERIPH_SIZE		UL(0x02000000)

/* SMC0 */
#define NRD_ROS_SMC0_BASE			UL(0x08000000)
#define NRD_ROS_SMC0_SIZE			UL(0x04000000)

#endif /* NRD_ROS_DEF3_H */
