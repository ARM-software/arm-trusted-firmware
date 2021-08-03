/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FSL_MMDC_H
#define FSL_MMDC_H

/* PHY Write Leveling Configuration and Error Status Register (MPWLGCR) */
#define MPWLGCR_HW_WL_EN		(1 << 0)

/* PHY Pre-defined Compare and CA delay-line Configuration (MPPDCMPR2) */
#define MPPDCMPR2_MPR_COMPARE_EN	(1 << 0)


/* MMDC PHY Read DQS gating control register 0 (MPDGCTRL0) */
#define AUTO_RD_DQS_GATING_CALIBRATION_EN	(1 << 28)

/* MMDC PHY Read Delay HW Calibration Control Register (MPRDDLHWCTL) */
#define MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN	(1 << 4)

/* MMDC Core Power Saving Control and Status Register (MMDC_MAPSR) */
#define MMDC_MAPSR_PWR_SAV_CTRL_STAT	0x00001067

/* MMDC Core Refresh Control Register (MMDC_MDREF) */
#define MDREF_START_REFRESH	(1 << 0)

/* MMDC Core Special Command Register (MDSCR) */
#define CMD_ADDR_MSB_MR_OP(x)	(x << 24)
#define CMD_ADDR_LSB_MR_ADDR(x)	(x << 16)
#define MDSCR_DISABLE_CFG_REQ	(0 << 15)
#define MDSCR_ENABLE_CON_REQ	(1 << 15)
#define MDSCR_CON_ACK		(1 << 14)
#define MDSCR_WL_EN		(1 << 9)
#define	CMD_NORMAL		(0 << 4)
#define	CMD_PRECHARGE		(1 << 4)
#define	CMD_AUTO_REFRESH	(2 << 4)
#define	CMD_LOAD_MODE_REG	(3 << 4)
#define	CMD_ZQ_CALIBRATION	(4 << 4)
#define	CMD_PRECHARGE_BANK_OPEN	(5 << 4)
#define	CMD_MRR			(6 << 4)
#define CMD_BANK_ADDR_0		0x0
#define CMD_BANK_ADDR_1		0x1
#define CMD_BANK_ADDR_2		0x2
#define CMD_BANK_ADDR_3		0x3
#define CMD_BANK_ADDR_4		0x4
#define CMD_BANK_ADDR_5		0x5
#define CMD_BANK_ADDR_6		0x6
#define CMD_BANK_ADDR_7		0x7

/* MMDC Core Control Register (MDCTL) */
#define MDCTL_SDE0		(U(1) << 31)
#define MDCTL_SDE1		(1 << 30)

/* MMDC PHY ZQ HW control register (MMDC_MPZQHWCTRL) */
#define MPZQHWCTRL_ZQ_HW_FORCE	(1 << 16)

/* MMDC PHY Measure Unit Register (MMDC_MPMUR0) */
#define MMDC_MPMUR0_FRC_MSR	(1 << 11)

/* MMDC PHY Read delay-lines Configuration Register (MMDC_MPRDDLCTL) */
/* default 64 for a quarter cycle delay */
#define MMDC_MPRDDLCTL_DEFAULT_DELAY	0x40404040

/* MMDC Registers */
struct mmdc_regs {
	unsigned int mdctl;
	unsigned int mdpdc;
	unsigned int mdotc;
	unsigned int mdcfg0;
	unsigned int mdcfg1;
	unsigned int mdcfg2;
	unsigned int mdmisc;
	unsigned int mdscr;
	unsigned int mdref;
	unsigned int res1[2];
	unsigned int mdrwd;
	unsigned int mdor;
	unsigned int mdmrr;
	unsigned int mdcfg3lp;
	unsigned int mdmr4;
	unsigned int mdasp;
	unsigned int res2[239];
	unsigned int maarcr;
	unsigned int mapsr;
	unsigned int maexidr0;
	unsigned int maexidr1;
	unsigned int madpcr0;
	unsigned int madpcr1;
	unsigned int madpsr0;
	unsigned int madpsr1;
	unsigned int madpsr2;
	unsigned int madpsr3;
	unsigned int madpsr4;
	unsigned int madpsr5;
	unsigned int masbs0;
	unsigned int masbs1;
	unsigned int res3[2];
	unsigned int magenp;
	unsigned int res4[239];
	unsigned int mpzqhwctrl;
	unsigned int mpzqswctrl;
	unsigned int mpwlgcr;
	unsigned int mpwldectrl0;
	unsigned int mpwldectrl1;
	unsigned int mpwldlst;
	unsigned int mpodtctrl;
	unsigned int mprddqby0dl;
	unsigned int mprddqby1dl;
	unsigned int mprddqby2dl;
	unsigned int mprddqby3dl;
	unsigned int mpwrdqby0dl;
	unsigned int mpwrdqby1dl;
	unsigned int mpwrdqby2dl;
	unsigned int mpwrdqby3dl;
	unsigned int mpdgctrl0;
	unsigned int mpdgctrl1;
	unsigned int mpdgdlst0;
	unsigned int mprddlctl;
	unsigned int mprddlst;
	unsigned int mpwrdlctl;
	unsigned int mpwrdlst;
	unsigned int mpsdctrl;
	unsigned int mpzqlp2ctl;
	unsigned int mprddlhwctl;
	unsigned int mpwrdlhwctl;
	unsigned int mprddlhwst0;
	unsigned int mprddlhwst1;
	unsigned int mpwrdlhwst0;
	unsigned int mpwrdlhwst1;
	unsigned int mpwlhwerr;
	unsigned int mpdghwst0;
	unsigned int mpdghwst1;
	unsigned int mpdghwst2;
	unsigned int mpdghwst3;
	unsigned int mppdcmpr1;
	unsigned int mppdcmpr2;
	unsigned int mpswdar0;
	unsigned int mpswdrdr0;
	unsigned int mpswdrdr1;
	unsigned int mpswdrdr2;
	unsigned int mpswdrdr3;
	unsigned int mpswdrdr4;
	unsigned int mpswdrdr5;
	unsigned int mpswdrdr6;
	unsigned int mpswdrdr7;
	unsigned int mpmur0;
	unsigned int mpwrcadl;
	unsigned int mpdccr;
};

struct fsl_mmdc_info {
	unsigned int mdctl;
	unsigned int mdpdc;
	unsigned int mdotc;
	unsigned int mdcfg0;
	unsigned int mdcfg1;
	unsigned int mdcfg2;
	unsigned int mdmisc;
	unsigned int mdref;
	unsigned int mdrwd;
	unsigned int mdor;
	unsigned int mdasp;
	unsigned int mpodtctrl;
	unsigned int mpzqhwctrl;
	unsigned int mprddlctl;
};

void mmdc_init(const struct fsl_mmdc_info *priv, uintptr_t nxp_ddr_addr);

#endif /* FSL_MMDC_H */
