/*
 * Copyright 2016-2019 NXP
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
#define SDRAM_CFG_BI			0x00000001

#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define SDRAM_CFG2_AP_EN		0x00000020
#define SDRAM_CFG2_ODT_ONLY_READ	2

#define SDRAM_CFG3_DDRC_RST		0x80000000

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

/* Mode Registers */
#define DDR_MR5_CA_PARITY_LAT_4_CLK	0x1 /* for DDR4-1600/1866/2133 */
#define DDR_MR5_CA_PARITY_LAT_5_CLK	0x2 /* for DDR4-2400 */

/* DEBUG_29 register */
#define DDR_TX_BD_DIS	(1 << 10) /* Transmit Bit Deskew Disable */

#define DDR_INIT_ADDR_EXT_UIA	(1 << 31)

/* Record of register values computed */
struct ddr_cfg_regs {
	struct {
		unsigned int bnds;
		unsigned int config;
		unsigned int config_2;
	} cs[DDRC_NUM_CS];
	unsigned int dec[10];
	unsigned int timing_cfg[10];
	unsigned int sdram_cfg[3];
	unsigned int sdram_mode[16];
	unsigned int md_cntl;
	unsigned int interval;
	unsigned int data_init;
	unsigned int clk_cntl;
	unsigned int init_addr;
	unsigned int init_ext_addr;
	unsigned int zq_cntl;
	unsigned int wrlvl_cntl[3];
	unsigned int ddr_sr_cntr;
	unsigned int sdram_rcw[6];
	unsigned int dq_map[4];
	unsigned int eor;
	unsigned int cdr[2];
	unsigned int err_disable;
	unsigned int err_int_en;
	unsigned int tx_cfg[4];
	unsigned int debug[64];
};

#endif /* DDR_REG_H */
