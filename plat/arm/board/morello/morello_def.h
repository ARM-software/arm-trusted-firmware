/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
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
#define MORELLO_SDS_PLATFORM_INFO_SIZE		U(18)
#define MORELLO_MAX_DDR_CAPACITY		U(0x1000000000)
#define MORELLO_MAX_SLAVE_COUNT			U(16)

/* SDS BL33 image information defines */
#define MORELLO_SDS_BL33_INFO_STRUCT_ID		U(9)
#define MORELLO_SDS_BL33_INFO_OFFSET		U(0)
#define MORELLO_SDS_BL33_INFO_SIZE		U(12)

/* Base address of non-secure SRAM where Platform information will be filled */
#define MORELLO_PLATFORM_INFO_BASE		UL(0x06000000)

/* DMC memory status registers */
#define MORELLO_DMC0_MEMC_STATUS_REG		0x4E000000
#define MORELLO_DMC1_MEMC_STATUS_REG		0x4E100000

#define MORELLO_DMC_MEMC_STATUS_MASK		U(7)

/* DMC memory command registers */
#define MORELLO_DMC0_MEMC_CMD_REG		0x4E000008
#define MORELLO_DMC1_MEMC_CMD_REG		0x4E100008

/* DMC ERR0CTLR0 registers */
#define MORELLO_DMC0_ERR0CTLR0_REG		0x4E000708
#define MORELLO_DMC1_ERR0CTLR0_REG		0x4E100708

/* DMC ECC in ERR0CTLR0 register */
#define MORELLO_DMC_ERR0CTLR0_ECC_EN		U(9)

/* DMC ERR2STATUS register */
#define MORELLO_DMC0_ERR2STATUS_REG		0x4E000790
#define MORELLO_DMC1_ERR2STATUS_REG		0x4E100790

/* DMC memory commands */
#define MORELLO_DMC_MEMC_CMD_CONFIG		U(0)
#define MORELLO_DMC_MEMC_CMD_READY		U(3)

#endif /* MORELLO_DEF_H */
