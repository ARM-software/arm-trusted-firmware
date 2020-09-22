/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_DEF_H
#define MORELLO_DEF_H

/* Non-secure SRAM MMU mapping */
#define MORELLO_NS_SRAM_BASE			UL(0x06000000)
#define MORELLO_NS_SRAM_SIZE			UL(0x00010000)
#define MORELLO_MAP_NS_SRAM			MAP_REGION_FLAT(	\
						MORELLO_NS_SRAM_BASE,	\
						MORELLO_NS_SRAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

/* SDS Platform information defines */
#define MORELLO_SDS_PLATFORM_INFO_STRUCT_ID	U(8)
#define MORELLO_SDS_PLATFORM_INFO_OFFSET	U(0)
#define MORELLO_SDS_PLATFORM_INFO_SIZE		U(4)
#define MORELLO_MAX_DDR_CAPACITY_GB		U(64)
#define MORELLO_MAX_SLAVE_COUNT			U(16)

/* SDS BL33 image information defines */
#define MORELLO_SDS_BL33_INFO_STRUCT_ID		U(9)
#define MORELLO_SDS_BL33_INFO_OFFSET		U(0)
#define MORELLO_SDS_BL33_INFO_SIZE		U(12)

/* Base address of non-secure SRAM where Platform information will be filled */
#define MORELLO_PLATFORM_INFO_BASE		UL(0x06008000)

#endif /* MORELLO_DEF_H */
