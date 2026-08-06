/*
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#include "socfpga_handoff.h"

/*
 * Agilex72 Clock Manager replaces the legacy Main/Peripheral
 * membus PLLs with three fractional-synthesis GPPLLs (GPPLL0/1/2). ATF BL2 is
 * the FSBL and owns bring-up (see config_clkmgr_handoff()). Rate model:
 * consumer_hz = golden gppll_c_output_hz / live clock-manager divider
 */

/* Clock Manager Registers */
#define CLKMGR_BASE				0x0915C000
#define CLKMGR_CTRL				0x00
#define CLKMGR_STAT				0x04
#define CLKMGR_TESTIOCTROL			0x08
#define CLKMGR_INTRGEN				0x0C
#define CLKMGR_INTRMSK				0x10
#define CLKMGR_INTRCLR				0x14
#define CLKMGR_INTRSTS				0x18
#define CLKMGR_INTRSTK				0x1C
#define CLKMGR_INTRRAW				0x20

#define CLKMGR(_reg)				(CLKMGR_BASE + (CLKMGR_##_reg))

#define CLKMGR_CTRL_BOOTMODE			BIT(0)
#define CLKMGR_STAT_BUSY_E_BUSY			0x1
#define CLKMGR_STAT_BUSY(x)			(((x) & 0x00000001) >> 0)
#define CLKMGR_STAT_BOOTMODE			BIT(24)
#define CLKMGR_STAT_BOOTCLKSRC			BIT(25)

/*
 * GPPLL golden C-output frequencies (Hz), from the DV preset. GPPLL0 =
 * peripheral (VCO 2000 MHz), GPPLL1 = MPU/DSU (1850 MHz), GPPLL2 = A720 cores
 * (2500 MHz). ATF uses these as the C-output parents; the SDM owns the VCO
 * config.
 */
#define AGILEX72_GPPLL0_C0_HZ			1000000000U	/* 1000 MHz */
#define AGILEX72_GPPLL0_C1_HZ			500000000U	/*  500 MHz */
#define AGILEX72_GPPLL0_C2_HZ			400000000U	/*  400 MHz */
#define AGILEX72_GPPLL0_C3_HZ			2000000000U	/* 2000 MHz */
#define AGILEX72_GPPLL1_C0_HZ			1850000000U	/* comp0/MPU */
#define AGILEX72_GPPLL1_C1_HZ			1850000000U	/* dsu       */
#define AGILEX72_GPPLL2_C0_HZ			2500000000U	/* core2     */
#define AGILEX72_GPPLL2_C1_HZ			2500000000U	/* core3     */

#define AGILEX72_OSC1_HZ			200000000U
#define AGILEX72_INTOSC_DIV2_HZ			500000000U
#define AGILEX72_F2S_FREE_HZ			100000000U

/* Derived golden consumer frequencies (Hz); reference values for validation. */
#define AGILEX72_MPU_HZ				AGILEX72_GPPLL1_C0_HZ	/* 1850 MHz */
#define AGILEX72_L4_MAIN_HZ			AGILEX72_GPPLL0_C1_HZ	/*  500 MHz */
#define AGILEX72_L4_MP_HZ			250000000U		/*  250 MHz */
#define AGILEX72_L4_SP_HZ			125000000U		/*  125 MHz */
#define AGILEX72_WDT_HZ				125000000U		/*  125 MHz */
#define AGILEX72_UART_HZ			125000000U		/*  125 MHz */
#define AGILEX72_EMAC_HZ			250000000U		/*  250 MHz */
#define AGILEX72_SDMMC_HZ			200000000U		/*  200 MHz */

/*
 * consumer register offsets (from CLKMGR_BASE). *_CTR: count divider in
 * bits[10:0] (divisor = count + 1), source select in bits[18:16]. *DIV:
 * power-of-two exponent (divisor = 1 << field).
 */
#define CLKMGR_NOC_FREE_CTR			0x104
#define CLKMGR_LSPNOC_FREE_CTR			0x108
#define CLKMGR_COMP0_CTR			0x0BC
#define CLKMGR_CORE2_CTR			0x0B8
#define CLKMGR_CORE3_CTR			0x0B4
#define CLKMGR_DSU_CTR				0x0C0
#define CLKMGR_CCU_FREE_CTR			0x0C4
#define CLKMGR_EMACA_CTR			0x128
#define CLKMGR_EMACB_CTR			0x12C
#define CLKMGR_EMACPTP_FREE_CTR			0x130
#define CLKMGR_USB31_FREE_CTR			0x140
#define CLKMGR_MEMDEVPHY_FREE_CTR		0x148
#define CLKMGR_XSPIPHY_FREE_CTR			0x154

#define CLKMGR_FREE_CTR_CNT_MASK		GENMASK(10, 0)
#define CLKMGR_FREE_CTR_SRC_SHIFT		16
#define CLKMGR_FREE_CTR_SRC_MASK		GENMASK(18, 16)

/* NoC power-of-two divider register (LSP/HSP sys/mp/sp, CoreSight) */
#define CLKMGR_NOCDIV				0x10C
#define CLKMGR_NOCDIV_LSPSYS_SHIFT		2
#define CLKMGR_NOCDIV_LSPMP_SHIFT		4
#define CLKMGR_NOCDIV_LSPSP_SHIFT		6
#define CLKMGR_NOCDIV_HSPSYS_SHIFT		8
#define CLKMGR_NOCDIV_HSPMP_SHIFT		10
#define CLKMGR_NOCDIV_HSPSP_SHIFT		12
#define CLKMGR_NOCDIV_WIDTH			2

/* Peripheral power-of-two divider register (SDMMC0/1, XSPI PHY) */
#define CLKMGR_PERIPCTR				0x110
#define CLKMGR_PERIPCTR_SDMMC0_SHIFT		4
#define CLKMGR_PERIPCTR_SDMMC1_SHIFT		6
#define CLKMGR_PERIPCTR_XSPIPHY_SHIFT		8
#define CLKMGR_PERIPCTR_WIDTH			2

/* Other power-of-two divider register (EMAC A/B, USB31) */
#define CLKMGR_OTHERDIV				0x118
#define CLKMGR_OTHERDIV_EMACA_SHIFT		8
#define CLKMGR_OTHERDIV_EMACB_SHIFT		10
#define CLKMGR_OTHERDIV_USB31_SHIFT		12

/*
 * A720 cluster clock enable (BL31, before PSCI core release). mainpllgrp.en
 * @ 0x30 gates comp0/dsu/ccu/core2/core3; extcntrst @ 0xA8 holds the divided
 * clocks in reset until released.
 */
#define CLKMGR_MAINPLL_EN			0x030
#define CLKMGR_EXTCNTRST			0x0A8

#define CLKMGR_MAINPLL_EN_CCU			BIT(6)
#define CLKMGR_MAINPLL_EN_DSU			BIT(7)
#define CLKMGR_MAINPLL_EN_COMP0			BIT(8)
#define CLKMGR_MAINPLL_EN_CORE2			BIT(10)
#define CLKMGR_MAINPLL_EN_CORE3			BIT(11)
#define CLKMGR_MAINPLL_EN_SMP_A720		(CLKMGR_MAINPLL_EN_CCU |	\
						 CLKMGR_MAINPLL_EN_DSU |	\
						 CLKMGR_MAINPLL_EN_COMP0 |	\
						 CLKMGR_MAINPLL_EN_CORE2 |	\
						 CLKMGR_MAINPLL_EN_CORE3)

#define CLKMGR_EXTCNTRST_CCU			BIT(9)
#define CLKMGR_EXTCNTRST_DSU			BIT(10)
#define CLKMGR_EXTCNTRST_COMP0			BIT(11)
#define CLKMGR_EXTCNTRST_CORE2			BIT(13)
#define CLKMGR_EXTCNTRST_CORE3			BIT(14)

#define CLKMGR_EXTCNTRST_CPU_RELEASE		(CLKMGR_EXTCNTRST_CCU |		\
						 CLKMGR_EXTCNTRST_DSU |		\
						 CLKMGR_EXTCNTRST_COMP0 |	\
						 CLKMGR_EXTCNTRST_CORE2 |	\
						 CLKMGR_EXTCNTRST_CORE3)

/*
 * GPPLL bring-up registers (used by config_clkmgr_handoff()): secure
 * unlock, per-PLL GLOB analog enable, STAT lock bits (8/10/12), and the
 * boot-bypass/counter-reset controls.
 */
#define CLKMGR_STAT_PLL0_LOCKED			BIT(8)
#define CLKMGR_STAT_PLL1_LOCKED			BIT(10)
#define CLKMGR_STAT_PLL2_LOCKED			BIT(12)
#define CLKMGR_STAT_ALL_LOCKED			(CLKMGR_STAT_PLL0_LOCKED |	\
						 CLKMGR_STAT_PLL1_LOCKED |	\
						 CLKMGR_STAT_PLL2_LOCKED)
#define CLKMGR_STAT_BUSY_BIT			BIT(0)

/* Per-PLL analog GLOB control apertures (offsets from CLKMGR_BASE) */
#define CLKMGR_PLL0_GLOB			0x160
#define CLKMGR_PLL1_GLOB			0x080
#define CLKMGR_PLL2_GLOB			0x060

/* 5-step GLOB enable/bias/unfreeze sequence (identical for all three PLLs) */
#define CLKMGR_PLLGLOB_S1_CSR_EN		0x00008026
#define CLKMGR_PLLGLOB_S2_BIAS_ON		0x00008036
#define CLKMGR_PLLGLOB_S3_FRZREG_OFF		0x00008016
#define CLKMGR_PLLGLOB_S4_IOTRI_ON		0x0000805E
#define CLKMGR_PLLGLOB_S5_FBLVDS_ON		0x000080DE

/* Secure-manager register that unlocks clock-manager writes */
#define AGILEX72_SECMGR_CLKMGR_UNLOCK_ADDR	0x09152068
#define AGILEX72_SECMGR_CLKMGR_UNLOCK_VAL	0x00000001

/* Peripheral externally-timed counter reset (release on boot-bypass exit) */
#define CLKMGR_PERICTL_EXTCNTRST		0x124
#define CLKMGR_PERICTL_EXTCNTRST_EMACA		BIT(0)
#define CLKMGR_PERICTL_EXTCNTRST_EMACB		BIT(1)
#define CLKMGR_PERICTL_EXTCNTRST_EMACPTP	BIT(2)
#define CLKMGR_PERICTL_EXTCNTRST_GPIODB		BIT(3)
#define CLKMGR_PERICTL_EXTCNTRST_TRACE		BIT(4)
#define CLKMGR_PERICTL_EXTCNTRST_S2FUSER0	BIT(5)
#define CLKMGR_PERICTL_EXTCNTRST_S2FUSER1	BIT(6)
#define CLKMGR_PERICTL_EXTCNTRST_USB31		BIT(8)
#define CLKMGR_PERICTL_EXTCNTRST_XSPIPHY	BIT(14)
#define CLKMGR_PERICTL_EXTCNTRST_MEMDEVPHY	BIT(15)
#define CLKMGR_PERICTL_EXTCNTRST_LSPNOC		BIT(18)

#define CLKMGR_PERICTL_EXTCNTRST_RELEASE	(				\
		CLKMGR_PERICTL_EXTCNTRST_EMACA |			\
		CLKMGR_PERICTL_EXTCNTRST_EMACB |			\
		CLKMGR_PERICTL_EXTCNTRST_EMACPTP |			\
		CLKMGR_PERICTL_EXTCNTRST_GPIODB |			\
		CLKMGR_PERICTL_EXTCNTRST_TRACE |			\
		CLKMGR_PERICTL_EXTCNTRST_S2FUSER0 |			\
		CLKMGR_PERICTL_EXTCNTRST_S2FUSER1 |			\
		CLKMGR_PERICTL_EXTCNTRST_USB31 |			\
		CLKMGR_PERICTL_EXTCNTRST_XSPIPHY |			\
		CLKMGR_PERICTL_EXTCNTRST_MEMDEVPHY |			\
		CLKMGR_PERICTL_EXTCNTRST_LSPNOC)

/* GPPLL lock poll bound (microseconds) */
#define CLKMGR_PLL_LOCK_TIMEOUT_US		50000
#define CLKMGR_BUSY_TIMEOUT_US			20000

/* Clock IDs consumed by ATF (unchanged ABI) */
#define CLKMGR_MPU_CLK_ID			0
#define CLKMGR_MPU_PERIPH_CLK_ID		1
#define CLKMGR_L4_MAIN_CLK_ID			2
#define CLKMGR_L4_MP_CLK_ID			3
#define CLKMGR_L4_SP_CLK_ID			4
#define CLKMGR_WDT_CLK_ID			5
#define CLKMGR_UART_CLK_ID			6
#define CLKMGR_EMAC0_CLK_ID			7
#define CLKMGR_EMAC1_CLK_ID			8
#define CLKMGR_EMAC2_CLK_ID			9
#define CLKMGR_EMAC_PTP_CLK_ID			10
#define CLKMGR_SDMMC_CLK_ID			11

int config_clkmgr_handoff(handoff *hoff_ptr);
uint32_t clkmgr_get_rate(uint32_t clk_id);

#endif
