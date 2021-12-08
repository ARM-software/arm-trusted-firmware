/*
 * Copyright (C) 2018-2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef PHY_COMPHY_3700_H
#define PHY_COMPHY_3700_H

#define PLL_SET_DELAY_US			600
#define COMPHY_PLL_TIMEOUT			1000
#define REG_16_BIT_MASK				0xFFFF

#define COMPHY_SELECTOR_PHY_REG			0xFC
/* bit0: 0: Lane1 is GbE0; 1: Lane1 is PCIE */
#define COMPHY_SELECTOR_PCIE_GBE0_SEL_BIT	BIT(0)
/* bit4: 0: Lane0 is GbE1; 1: Lane0 is USB3 */
#define COMPHY_SELECTOR_USB3_GBE1_SEL_BIT	BIT(4)
/* bit8: 0: Lane0 is USB3 instead of GbE1, Lane2 is SATA; 1: Lane2 is USB3 */
#define COMPHY_SELECTOR_USB3_PHY_SEL_BIT	BIT(8)

/* SATA PHY register offset */
#define SATAPHY_LANE2_REG_BASE_OFFSET		0x200

/* USB3 PHY offset compared to SATA PHY */
#define USB3PHY_LANE2_REG_BASE_OFFSET		0x200

/* Comphy lane2 indirect access register offset */
#define COMPHY_LANE2_INDIR_ADDR_OFFSET		0x0
#define COMPHY_LANE2_INDIR_DATA_OFFSET		0x4

/* PHY shift to get related register address */
enum {
	PCIE = 1,
	USB3,
};

#define PCIEPHY_SHFT		2
#define USB3PHY_SHFT		2
#define PHY_SHFT(unit)		((unit == PCIE) ? PCIEPHY_SHFT : USB3PHY_SHFT)

/* PHY register */
#define COMPHY_POWER_PLL_CTRL		0x01
#define PWR_PLL_CTRL_ADDR(unit)		(COMPHY_POWER_PLL_CTRL * PHY_SHFT(unit))
#define PU_IVREF_BIT			BIT(15)
#define PU_PLL_BIT			BIT(14)
#define PU_RX_BIT			BIT(13)
#define PU_TX_BIT			BIT(12)
#define PU_TX_INTP_BIT			BIT(11)
#define PU_DFE_BIT			BIT(10)
#define RESET_DTL_RX_BIT		BIT(9)
#define PLL_LOCK_BIT			BIT(8)
#define REF_FREF_SEL_OFFSET		0
#define REF_FREF_SEL_MASK		(0x1F << REF_FREF_SEL_OFFSET)
#define REF_FREF_SEL_SERDES_25MHZ	(0x1 << REF_FREF_SEL_OFFSET)
#define REF_FREF_SEL_SERDES_40MHZ	(0x3 << REF_FREF_SEL_OFFSET)
#define REF_FREF_SEL_SERDES_50MHZ	(0x4 << REF_FREF_SEL_OFFSET)
#define REF_FREF_SEL_PCIE_USB3_25MHZ	(0x2 << REF_FREF_SEL_OFFSET)
#define REF_FREF_SEL_PCIE_USB3_40MHZ	(0x3 << REF_FREF_SEL_OFFSET)
#define PHY_MODE_OFFSET			5
#define PHY_MODE_MASK			(7 << PHY_MODE_OFFSET)
#define PHY_MODE_SATA			(0x0 << PHY_MODE_OFFSET)
#define PHY_MODE_PCIE			(0x3 << PHY_MODE_OFFSET)
#define PHY_MODE_SGMII			(0x4 << PHY_MODE_OFFSET)
#define PHY_MODE_USB3			(0x5 << PHY_MODE_OFFSET)

#define COMPHY_KVCO_CAL_CTRL		0x02
#define KVCO_CAL_CTRL_ADDR(unit)	(COMPHY_KVCO_CAL_CTRL * PHY_SHFT(unit))
#define USE_MAX_PLL_RATE_BIT		BIT(12)
#define SPEED_PLL_OFFSET		2
#define SPEED_PLL_MASK			(0x3F << SPEED_PLL_OFFSET)
#define SPEED_PLL_VALUE_16		(0x10 << SPEED_PLL_OFFSET)

#define COMPHY_DIG_LOOPBACK_EN		0x23
#define DIG_LOOPBACK_EN_ADDR(unit)	(COMPHY_DIG_LOOPBACK_EN * \
					 PHY_SHFT(unit))
#define SEL_DATA_WIDTH_OFFSET		10
#define SEL_DATA_WIDTH_MASK		(0x3 << SEL_DATA_WIDTH_OFFSET)
#define DATA_WIDTH_10BIT		(0x0 << SEL_DATA_WIDTH_OFFSET)
#define DATA_WIDTH_20BIT		(0x1 << SEL_DATA_WIDTH_OFFSET)
#define DATA_WIDTH_40BIT		(0x2 << SEL_DATA_WIDTH_OFFSET)
#define PLL_READY_TX_BIT		BIT(4)

#define COMPHY_SYNC_PATTERN		0x24
#define SYNC_PATTERN_ADDR(unit)		(COMPHY_SYNC_PATTERN * PHY_SHFT(unit))
#define TXD_INVERT_BIT			BIT(10)
#define RXD_INVERT_BIT			BIT(11)

#define COMPHY_SYNC_MASK_GEN		0x25
#define PHY_GEN_MAX_OFFSET		10
#define PHY_GEN_MAX_MASK		(3 << PHY_GEN_MAX_OFFSET)
#define PHY_GEN_MAX_USB3_5G		(1 << PHY_GEN_MAX_OFFSET)

#define COMPHY_ISOLATION_CTRL		0x26
#define ISOLATION_CTRL_ADDR(unit)	(COMPHY_ISOLATION_REG * PHY_SHFT(unit))
#define PHY_ISOLATE_MODE		BIT(15)

#define COMPHY_GEN2_SET2		0x3e
#define GEN2_SET2_ADDR(unit)		(COMPHY_GEN2_SET2 * PHY_SHFT(unit))
#define GS2_TX_SSC_AMP_VALUE_20		BIT(14)
#define GS2_TX_SSC_AMP_OFF		9
#define GS2_TX_SSC_AMP_LEN		7
#define GS2_TX_SSC_AMP_MASK		(((1 << GS2_TX_SSC_AMP_LEN) - 1) << \
					 GS2_TX_SSC_AMP_OFF)
#define GS2_VREG_RXTX_MAS_ISET_OFF	7
#define GS2_VREG_RXTX_MAS_ISET_60U	(0 << GS2_VREG_RXTX_MAS_ISET_OFF)
#define GS2_VREG_RXTX_MAS_ISET_80U	(1 << GS2_VREG_RXTX_MAS_ISET_OFF)
#define GS2_VREG_RXTX_MAS_ISET_100U	(2 << GS2_VREG_RXTX_MAS_ISET_OFF)
#define GS2_VREG_RXTX_MAS_ISET_120U	(3 << GS2_VREG_RXTX_MAS_ISET_OFF)
#define GS2_VREG_RXTX_MAS_ISET_MASK	(BIT(7) | BIT(8))
#define GS2_RSVD_6_0_OFF		0
#define GS2_RSVD_6_0_LEN		7
#define GS2_RSVD_6_0_MASK		(((1 << GS2_RSVD_6_0_LEN) - 1) << \
					 GS2_RSVD_6_0_OFF)

#define COMPHY_GEN3_SET2		0x3f
#define GEN3_SET2_ADDR(unit)		(COMPHY_GEN3_SET2 * PHY_SHFT(unit))

#define COMPHY_IDLE_SYNC_EN		0x48
#define IDLE_SYNC_EN_ADDR(unit)		(COMPHY_IDLE_SYNC_EN * PHY_SHFT(unit))
#define IDLE_SYNC_EN			BIT(12)
#define IDLE_SYNC_EN_DEFAULT_VALUE	0x60

#define COMPHY_MISC_CTRL0		0x4F
#define MISC_CTRL0_ADDR(unit)		(COMPHY_MISC_CTRL0 * PHY_SHFT(unit))
#define CLK100M_125M_EN			BIT(4)
#define TXDCLK_2X_SEL			BIT(6)
#define CLK500M_EN			BIT(7)
#define PHY_REF_CLK_SEL			BIT(10)
#define MISC_CTRL0_DEFAULT_VALUE	0xA00D

#define COMPHY_MISC_CTRL1		0x73
#define MISC_CTRL1_ADDR(unit)		(COMPHY_MISC_CTRL1 * PHY_SHFT(unit))
#define SEL_BITS_PCIE_FORCE		BIT(15)

#define COMPHY_GEN2_SET3		0x112
#define GS3_FFE_CAP_SEL_MASK		0xF
#define GS3_FFE_CAP_SEL_VALUE		0xF

#define COMPHY_LANE_CFG0		0x180
#define LANE_CFG0_ADDR(unit)		(COMPHY_LANE_CFG0 * PHY_SHFT(unit))
#define PRD_TXDEEMPH0_MASK		BIT(0)
#define PRD_TXMARGIN_MASK		(BIT(1) | BIT(2) | BIT(3))
#define PRD_TXSWING_MASK		BIT(4)
#define CFG_TX_ALIGN_POS_MASK		(BIT(5) | BIT(6) | BIT(7) | BIT(8))

#define COMPHY_LANE_CFG1		0x181
#define LANE_CFG1_ADDR(unit)		(COMPHY_LANE_CFG1 * PHY_SHFT(unit))
#define PRD_TXDEEMPH1_MASK		BIT(15)
#define USE_MAX_PLL_RATE_EN		BIT(9)
#define TX_DET_RX_MODE			BIT(6)
#define GEN2_TX_DATA_DLY_MASK		(BIT(3) | BIT(4))
#define GEN2_TX_DATA_DLY_DEFT		(2 << 3)
#define TX_ELEC_IDLE_MODE_EN		BIT(0)

#define COMPHY_LANE_STAT1		0x183
#define LANE_STAT1_ADDR(unit)		(COMPHY_LANE_STAT1 * PHY_SHFT(unit))
#define TXDCLK_PCLK_EN				BIT(0)

#define COMPHY_LANE_CFG4		0x188
#define LANE_CFG4_ADDR(unit)		(COMPHY_LANE_CFG4 * PHY_SHFT(unit))
#define SPREAD_SPECTRUM_CLK_EN		BIT(7)

#define COMPHY_RST_CLK_CTRL		0x1C1
#define RST_CLK_CTRL_ADDR(unit)		(COMPHY_RST_CLK_CTRL * PHY_SHFT(unit))
#define SOFT_RESET			BIT(0)
#define MODE_CORE_CLK_FREQ_SEL		BIT(9)
#define MODE_PIPE_WIDTH_32		BIT(3)
#define MODE_REFDIV_OFFSET		4
#define MODE_REFDIV_LEN			2
#define MODE_REFDIV_MASK		(0x3 << MODE_REFDIV_OFFSET)
#define MODE_REFDIV_BY_4		(0x2 << MODE_REFDIV_OFFSET)

#define COMPHY_TEST_MODE_CTRL		0x1C2
#define TEST_MODE_CTRL_ADDR(unit)	(COMPHY_TEST_MODE_CTRL * PHY_SHFT(unit))
#define MODE_MARGIN_OVERRIDE		BIT(2)

#define COMPHY_CLK_SRC_LO		0x1C3
#define CLK_SRC_LO_ADDR(unit)		(COMPHY_CLK_SRC_LO * PHY_SHFT(unit))
#define MODE_CLK_SRC			BIT(0)
#define BUNDLE_PERIOD_SEL		BIT(1)
#define BUNDLE_PERIOD_SCALE_MASK	(BIT(2) | BIT(3))
#define BUNDLE_SAMPLE_CTRL		BIT(4)
#define PLL_READY_DLY_MASK		(BIT(5) | BIT(6) | BIT(7))
#define CFG_SEL_20B			BIT(15)

#define COMPHY_PWR_MGM_TIM1		0x1D0
#define PWR_MGM_TIM1_ADDR(unit)		(COMPHY_PWR_MGM_TIM1 * PHY_SHFT(unit))
#define CFG_PM_OSCCLK_WAIT_OFF		12
#define CFG_PM_OSCCLK_WAIT_LEN		4
#define CFG_PM_OSCCLK_WAIT_MASK		(((1 << CFG_PM_OSCCLK_WAIT_LEN) - 1) \
					 << CFG_PM_OSCCLK_WAIT_OFF)
#define CFG_PM_RXDEN_WAIT_OFF		8
#define CFG_PM_RXDEN_WAIT_LEN		4
#define CFG_PM_RXDEN_WAIT_MASK		(((1 << CFG_PM_RXDEN_WAIT_LEN) - 1) \
					 << CFG_PM_RXDEN_WAIT_OFF)
#define CFG_PM_RXDEN_WAIT_1_UNIT	(1 << CFG_PM_RXDEN_WAIT_OFF)
#define CFG_PM_RXDLOZ_WAIT_OFF		0
#define CFG_PM_RXDLOZ_WAIT_LEN		8
#define CFG_PM_RXDLOZ_WAIT_MASK		(((1 << CFG_PM_RXDLOZ_WAIT_LEN) - 1) \
					 << CFG_PM_RXDLOZ_WAIT_OFF)
#define CFG_PM_RXDLOZ_WAIT_7_UNIT	(7 << CFG_PM_RXDLOZ_WAIT_OFF)
#define CFG_PM_RXDLOZ_WAIT_12_UNIT	(0xC << CFG_PM_RXDLOZ_WAIT_OFF)

/*
 * This register is not from PHY lane register space. It only exists in the
 * indirect register space, before the actual PHY lane 2 registers. So the
 * offset is absolute, not relative to SATAPHY_LANE2_REG_BASE_OFFSET.
 * It is used only for SATA PHY initialization.
 */
#define COMPHY_RESERVED_REG		0x0E
#define PHYCTRL_FRM_PIN_BIT		BIT(13)

/* SGMII */
#define COMPHY_PHY_CFG1_OFFSET(lane)	((1 - (lane)) * 0x28)
#define PIN_PU_IVREF_BIT		BIT(1)
#define PIN_RESET_CORE_BIT		BIT(11)
#define PIN_RESET_COMPHY_BIT		BIT(12)
#define PIN_PU_PLL_BIT			BIT(16)
#define PIN_PU_RX_BIT			BIT(17)
#define PIN_PU_TX_BIT			BIT(18)
#define PIN_TX_IDLE_BIT			BIT(19)
#define GEN_RX_SEL_OFFSET		22
#define GEN_RX_SEL_MASK			(0xF << GEN_RX_SEL_OFFSET)
#define GEN_TX_SEL_OFFSET		26
#define GEN_TX_SEL_MASK			(0xF << GEN_TX_SEL_OFFSET)
#define PHY_RX_INIT_BIT			BIT(30)
#define SD_SPEED_1_25_G			0x6
#define SD_SPEED_3_125_G		0x8

/* COMPHY status reg:
 * lane0: USB3/GbE1 PHY Status 1
 * lane1: PCIe/GbE0 PHY Status 1
 */
#define COMPHY_PHY_STATUS_OFFSET(lane)	(0x18 + (1 - (lane)) * 0x28)
#define PHY_RX_INIT_DONE_BIT		BIT(0)
#define PHY_PLL_READY_RX_BIT		BIT(2)
#define PHY_PLL_READY_TX_BIT		BIT(3)

#define SGMIIPHY_ADDR(off, base)	((((off) & 0x00007FF) * 2) + (base))

#define MAX_LANE_NR			3

/* comphy API */
int mvebu_3700_comphy_is_pll_locked(uint8_t comphy_index, uint32_t comphy_mode);
int mvebu_3700_comphy_power_off(uint8_t comphy_index, uint32_t comphy_mode);
int mvebu_3700_comphy_power_on(uint8_t comphy_index, uint32_t comphy_mode);
#endif /* PHY_COMPHY_3700_H */
