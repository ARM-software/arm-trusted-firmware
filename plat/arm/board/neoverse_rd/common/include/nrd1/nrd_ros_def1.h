/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the RoS specific definitions for the first
 * generation platforms based on the A75, N1 and V1 CPUs. RoS (Rest Of System)
 * is used to refer to the part of the reference design platform that excludes
 * CSS.
 */

#ifndef NRD_ROS_DEF1_H
#define NRD_ROS_DEF1_H

/*******************************************************************************
 * ROS configs
 ******************************************************************************/

/* RoS Peripherals */
#define NRD_ROS_PERIPH_BASE		UL(0x60000000)
#define NRD_ROS_PERIPH_SIZE		UL(0x20000000)

/* System Reg */
#define NRD_ROS_SYSTEMREG_BASE		UL(0x1C010000)
#define NRD_ROS_SYSTEMREG_SIZE		UL(0x00010000)

/* NOR Flash 2 */
#define NRD_ROS_NOR2_FLASH_BASE		UL(0x10000000)
#define NRD_ROS_NOR2_FLASH_SIZE		UL(0x04000000)

/* RoS Platform */
#define NRD_ROS_PLATFORM_BASE		UL(0x7F000000)
#define NRD_ROS_PLATFORM_SIZE		UL(0x20000000)

#endif /* NRD_ROS_DEF1_H */
