/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define RCAR_DDR_VERSION	"rev.0.41"
#define DRAM_CH_CNT		0x04
#define SLICE_CNT		0x04
#define CS_CNT			0x02

/* order : CS0A, CS0B, CS1A, CS1B */
#define CSAB_CNT		(CS_CNT * 2)

/* order : CH0A, CH0B, CH1A, CH1B, CH2A, CH2B, CH3A, CH3B */
#define CHAB_CNT		(DRAM_CH_CNT * 2)

/* pll setting */
#define CLK_DIV(a, diva, b, divb) (((a) * (divb)) / ((b) * (diva)))
#define CLK_MUL(a, diva, b, divb) (((a) * (b)) / ((diva) * (divb)))

/* for ddr deisity setting */
#define DBMEMCONF_REG(d3, row, bank, col, dw)	\
	(((d3) << 30) | ((row) << 24) | ((bank) << 16) | ((col) << 8) | (dw))

#define DBMEMCONF_REGD(density)		\
	(DBMEMCONF_REG((density) % 2, ((density) + 1) / \
	2 + (29 - 3 - 10 - 2), 3, 10, 2))

#define DBMEMCONF_VAL(ch, cs) (DBMEMCONF_REGD(DBMEMCONF_DENS(ch, cs)))

/* refresh mode */
#define DBSC_REFINTS		(0x0)

/* system registers */
#define CPG_FRQCRB		(CPG_BASE + 0x0004U)

#define CPG_PLLECR		(CPG_BASE + 0x00D0U)
#define CPG_MSTPSR5		(CPG_BASE + 0x003CU)
#define CPG_SRCR4		(CPG_BASE + 0x00BCU)
#define CPG_PLL3CR		(CPG_BASE + 0x00DCU)
#define CPG_ZB3CKCR		(CPG_BASE + 0x0380U)
#define CPG_FRQCRD		(CPG_BASE + 0x00E4U)
#define CPG_SMSTPCR5		(CPG_BASE + 0x0144U)
#define CPG_CPGWPR		(CPG_BASE + 0x0900U)
#define CPG_SRSTCLR4		(CPG_BASE + 0x0950U)

#define CPG_FRQCRB_KICK_BIT	BIT(31)
#define CPG_PLLECR_PLL3E_BIT	BIT(3)
#define CPG_PLLECR_PLL3ST_BIT	BIT(11)
#define CPG_ZB3CKCR_ZB3ST_BIT	BIT(11)

#define RST_BASE		(0xE6160000U)
#define RST_MODEMR		(RST_BASE + 0x0060U)

#define LIFEC_CHIPID(x)		(0xE6110040U + 0x04U * (x))

/* DBSC registers */
#include "../ddr_regs.h"

#define DBSC_DBMONCONF4		0xE6793010U

#define DBSC_PLL_LOCK(ch)	(0xE6794054U + 0x100U * (ch))
#define DBSC_PLL_LOCK_0		0xE6794054U
#define DBSC_PLL_LOCK_1		0xE6794154U
#define DBSC_PLL_LOCK_2		0xE6794254U
#define DBSC_PLL_LOCK_3		0xE6794354U

/* STAT registers */
#define MSTAT_SL_INIT		0xE67E8000U
#define MSTAT_REF_ARS		0xE67E8004U
#define MSTATQ_STATQC		0xE67E8008U
#define MSTATQ_WTENABLE		0xE67E8030U
#define MSTATQ_WTREFRESH	0xE67E8034U
#define MSTATQ_WTSETTING0	0xE67E8038U
#define MSTATQ_WTSETTING1	0xE67E803CU

#define QOS_BASE1		(0xE67F0000U)
#define QOSCTRL_RAS		(QOS_BASE1 + 0x0000U)
#define QOSCTRL_FIXTH		(QOS_BASE1 + 0x0004U)
#define QOSCTRL_RAEN		(QOS_BASE1 + 0x0018U)
#define QOSCTRL_REGGD		(QOS_BASE1 + 0x0020U)
#define QOSCTRL_DANN		(QOS_BASE1 + 0x0030U)
#define QOSCTRL_DANT		(QOS_BASE1 + 0x0038U)
#define QOSCTRL_EC		(QOS_BASE1 + 0x003CU)
#define QOSCTRL_EMS		(QOS_BASE1 + 0x0040U)
#define QOSCTRL_INSFC		(QOS_BASE1 + 0x0050U)
#define QOSCTRL_BERR		(QOS_BASE1 + 0x0054U)
#define QOSCTRL_RACNT0		(QOS_BASE1 + 0x0080U)
#define QOSCTRL_STATGEN0	(QOS_BASE1 + 0x0088U)

/* other module */
#define THS1_THCTR		0xE6198020U
#define THS1_TEMP		0xE6198028U
