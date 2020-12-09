/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_REG_H
#define DDR_REG_H

#define SDRAM_CS_CONFIG_EN		0x80000000

/* DDR_SDRAM_CFG - DDR SDRAM Control Configuration
 */
#define SDRAM_CFG_MEM_EN		0x80000000
#define SDRAM_CFG_SREN			0x40000000
#define SDRAM_CFG_ECC_EN		0x20000000
#define SDRAM_CFG_RD_EN			0x10000000
#define SDRAM_CFG_SDRAM_TYPE_MASK	0x07000000
#define SDRAM_CFG_SDRAM_TYPE_SHIFT	24
#define SDRAM_CFG_DYN_PWR		0x00200000
#define SDRAM_CFG_DBW_MASK		0x00180000
#define SDRAM_CFG_DBW_SHIFT		19
#define SDRAM_CFG_32_BW			0x00080000
#define SDRAM_CFG_16_BW			0x00100000
#define SDRAM_CFG_8_BW			0x00180000
#define SDRAM_CFG_8_BE			0x00040000
#define SDRAM_CFG_2T_EN			0x00008000
#define SDRAM_CFG_MEM_HLT		0x00000002
#define SDRAM_CFG_BI			0x00000001

#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG2_FRC_SR_CLEAR		~(SDRAM_CFG2_FRC_SR)
#define SDRAM_CFG2_D_INIT		0x00000010
#define SDRAM_CFG2_AP_EN		0x00000020
#define SDRAM_CFG2_ODT_ONLY_READ	2

#define SDRAM_CFG3_DDRC_RST		0x80000000

#define SDRAM_INTERVAL_REFINT	0xFFFF0000
#define SDRAM_INTERVAL_REFINT_CLEAR	~(SDRAM_INTERVAL_REFINT)
#define SDRAM_INTERVAL_BSTOPRE	0x3FFF

/* DDR_MD_CNTL */
#define MD_CNTL_MD_EN		0x80000000
#define MD_CNTL_CS_SEL(x)	(((x) & 0x7) << 28)
#define MD_CNTL_MD_SEL(x)	(((x) & 0xf) << 24)
#define MD_CNTL_CKE(x)		(((x) & 0x3) << 20)

/* DDR_CDR1 */
#define DDR_CDR1_DHC_EN	0x80000000
#define DDR_CDR1_ODT_SHIFT	17
#define DDR_CDR1_ODT_MASK	0x6
#define DDR_CDR2_ODT_MASK	0x1
#define DDR_CDR1_ODT(x) ((x & DDR_CDR1_ODT_MASK) << DDR_CDR1_ODT_SHIFT)
#define DDR_CDR2_ODT(x) (x & DDR_CDR2_ODT_MASK)
#define DDR_CDR2_VREF_OVRD(x)	(0x00008080 | ((((x) - 37) & 0x3F) << 8))
#define DDR_CDR2_VREF_TRAIN_EN	0x00000080
#define DDR_CDR2_VREF_RANGE_2	0x00000040
#define DDR_CDR_ODT_OFF		0x0
#define DDR_CDR_ODT_100ohm	0x1
#define DDR_CDR_ODT_120OHM	0x2
#define DDR_CDR_ODT_80ohm	0x3
#define DDR_CDR_ODT_60ohm	0x4
#define DDR_CDR_ODT_40ohm	0x5
#define DDR_CDR_ODT_50ohm	0x6
#define DDR_CDR_ODT_30ohm	0x7


/* DDR ERR_DISABLE */
#define DDR_ERR_DISABLE_APED	(1 << 8)  /* Address parity error disable */
#define DDR_ERR_DISABLE_SBED	(1 << 2)  /* Address parity error disable */
#define DDR_ERR_DISABLE_MBED	(1 << 3)  /* Address parity error disable */

/* Mode Registers */
#define DDR_MR5_CA_PARITY_LAT_4_CLK	0x1 /* for DDR4-1600/1866/2133 */
#define DDR_MR5_CA_PARITY_LAT_5_CLK	0x2 /* for DDR4-2400 */

/* DDR DSR2  register */
#define DDR_DSR_2_PHY_INIT_CMPLT	0x4

/* SDRAM TIMING_CFG_10 register */
#define DDR_TIMING_CFG_10_T_STAB	0x7FFF

/* DEBUG 2 register */
#define DDR_DBG_2_MEM_IDLE		0x00000002

/* DEBUG 26 register */
#define DDR_DEBUG_26_BIT_6		(0x1 << 6)
#define DDR_DEBUG_26_BIT_7		(0x1 << 7)
#define DDR_DEBUG_26_BIT_12		(0x1 << 12)
#define DDR_DEBUG_26_BIT_13		(0x1 << 13)
#define DDR_DEBUG_26_BIT_14		(0x1 << 14)
#define DDR_DEBUG_26_BIT_15		(0x1 << 15)
#define DDR_DEBUG_26_BIT_16		(0x1 << 16)
#define DDR_DEBUG_26_BIT_17		(0x1 << 17)
#define DDR_DEBUG_26_BIT_18		(0x1 << 18)
#define DDR_DEBUG_26_BIT_19		(0x1 << 19)
#define DDR_DEBUG_26_BIT_24		(0x1 << 24)
#define DDR_DEBUG_26_BIT_25		(0x1 << 25)

#define DDR_DEBUG_26_BIT_24_CLEAR	~(DDR_DEBUG_26_BIT_24)

/* DEBUG_29 register */
#define DDR_TX_BD_DIS	(1 << 10) /* Transmit Bit Deskew Disable */

#define DDR_INIT_ADDR_EXT_UIA	(1 << 31)

#endif /* DDR_REG_H */
