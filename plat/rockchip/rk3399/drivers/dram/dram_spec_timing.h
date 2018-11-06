/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRAM_SPEC_TIMING_H
#define DRAM_SPEC_TIMING_H

#include <stdint.h>

enum ddr3_speed_rate {
	/* 5-5-5 */
	DDR3_800D = 0,
	/* 6-6-6 */
	DDR3_800E = 1,
	/* 6-6-6 */
	DDR3_1066E = 2,
	/* 7-7-7 */
	DDR3_1066F = 3,
	/* 8-8-8 */
	DDR3_1066G = 4,
	/* 7-7-7 */
	DDR3_1333F = 5,
	/* 8-8-8 */
	DDR3_1333G = 6,
	/* 9-9-9 */
	DDR3_1333H = 7,
	/* 10-10-10 */
	DDR3_1333J = 8,
	/* 8-8-8 */
	DDR3_1600G = 9,
	/* 9-9-9 */
	DDR3_1600H = 10,
	/* 10-10-10 */
	DDR3_1600J = 11,
	/* 11-11-11 */
	DDR3_1600K = 12,
	/* 10-10-10 */
	DDR3_1866J = 13,
	/* 11-11-11 */
	DDR3_1866K = 14,
	/* 12-12-12 */
	DDR3_1866L = 15,
	/* 13-13-13 */
	DDR3_1866M = 16,
	/* 11-11-11 */
	DDR3_2133K = 17,
	/* 12-12-12 */
	DDR3_2133L = 18,
	/* 13-13-13 */
	DDR3_2133M = 19,
	/* 14-14-14 */
	DDR3_2133N = 20,
	DDR3_DEFAULT = 21,
};

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define range(mi, val, ma)  (((ma) > (val)) ? (max(mi, val)) : (ma))

struct dram_timing_t {
	/* unit MHz */
	uint32_t mhz;
	/* some timing unit is us */
	uint32_t tinit1;
	uint32_t tinit2;
	uint32_t tinit3;
	uint32_t tinit4;
	uint32_t tinit5;
	/* reset low, DDR3:200us */
	uint32_t trstl;
	/* reset high to CKE high, DDR3:500us  */
	uint32_t trsth;
	uint32_t trefi;
	/* base */
	uint32_t trcd;
	/* trp per bank */
	uint32_t trppb;
	/* trp all bank */
	uint32_t trp;
	uint32_t twr;
	uint32_t tdal;
	uint32_t trtp;
	uint32_t trc;
	uint32_t trrd;
	uint32_t tccd;
	uint32_t twtr;
	uint32_t trtw;
	uint32_t tras_max;
	uint32_t tras_min;
	uint32_t tfaw;
	uint32_t trfc;
	uint32_t tdqsck;
	uint32_t tdqsck_max;
	/* pd or sr */
	uint32_t txsr;
	uint32_t txsnr;
	uint32_t txp;
	uint32_t txpdll;
	uint32_t tdllk;
	uint32_t tcke;
	uint32_t tckesr;
	uint32_t tcksre;
	uint32_t tcksrx;
	uint32_t tdpd;
	/* mode regiter timing */
	uint32_t tmod;
	uint32_t tmrd;
	uint32_t tmrr;
	uint32_t tmrri;
	/* ODT */
	uint32_t todton;
	/* ZQ */
	uint32_t tzqinit;
	uint32_t tzqcs;
	uint32_t tzqoper;
	uint32_t tzqreset;
	/* Write Leveling */
	uint32_t twlmrd;
	uint32_t twlo;
	uint32_t twldqsen;
	/* CA Training */
	uint32_t tcackel;
	uint32_t tcaent;
	uint32_t tcamrd;
	uint32_t tcackeh;
	uint32_t tcaext;
	uint32_t tadr;
	uint32_t tmrz;
	uint32_t tcacd;
	/* mode register */
	uint32_t mr[4];
	uint32_t mr11;
	/* lpddr4 spec */
	uint32_t mr12;
	uint32_t mr13;
	uint32_t mr14;
	uint32_t mr16;
	uint32_t mr17;
	uint32_t mr20;
	uint32_t mr22;
	uint32_t tccdmw;
	uint32_t tppd;
	uint32_t tescke;
	uint32_t tsr;
	uint32_t tcmdcke;
	uint32_t tcscke;
	uint32_t tckelcs;
	uint32_t tcsckeh;
	uint32_t tckehcs;
	uint32_t tmrwckel;
	uint32_t tzqcal;
	uint32_t tzqlat;
	uint32_t tzqcke;
	uint32_t tvref_long;
	uint32_t tvref_short;
	uint32_t tvrcg_enable;
	uint32_t tvrcg_disable;
	uint32_t tfc_long;
	uint32_t tckfspe;
	uint32_t tckfspx;
	uint32_t tckehcmd;
	uint32_t tckelcmd;
	uint32_t tckelpd;
	uint32_t tckckel;
	/* other */
	uint32_t al;
	uint32_t cl;
	uint32_t cwl;
	uint32_t bl;
};

struct dram_info_t {
	/* speed_rate only used when DDR3 */
	enum ddr3_speed_rate speed_rate;
	/* 1: use CS0, 2: use CS0 and CS1 */
	uint32_t cs_cnt;
	/* give the max per-die capability on each rank/cs */
	uint32_t per_die_capability[2];
};

struct timing_related_config {
	struct dram_info_t dram_info[2];
	uint32_t dram_type;
	/* MHz */
	uint32_t freq;
	uint32_t ch_cnt;
	uint32_t bl;
	/* 1:auto precharge, 0:never auto precharge */
	uint32_t ap;
	/*
	 * 1:dll bypass, 0:dll normal
	 * dram and controller dll bypass at the same time
	 */
	uint32_t dllbp;
	/* 1:odt enable, 0:odt disable */
	uint32_t odt;
	/* 1:enable, 0:disabe */
	uint32_t rdbi;
	uint32_t wdbi;
	/* dram driver strength */
	uint32_t dramds;
	/* dram ODT, if odt=0, this parameter invalid */
	uint32_t dramodt;
	/*
	 * ca ODT, if odt=0, this parameter invalid
	 * it only used by LPDDR4
	 */
	uint32_t caodt;
};

/* mr0 for ddr3 */
#define DDR3_BL8		(0)
#define DDR3_BC4_8		(1)
#define DDR3_BC4		(2)
#define DDR3_CL(n)		(((((n) - 4) & 0x7) << 4)\
				| ((((n) - 4) & 0x8) >> 1))
#define DDR3_WR(n)		(((n) & 0x7) << 9)
#define DDR3_DLL_RESET		(1 << 8)
#define DDR3_DLL_DERESET	(0 << 8)

/* mr1 for ddr3 */
#define DDR3_DLL_ENABLE		(0)
#define DDR3_DLL_DISABLE	(1)
#define DDR3_MR1_AL(n)		(((n) & 0x3) << 3)

#define DDR3_DS_40		(0)
#define DDR3_DS_34		(1 << 1)
#define DDR3_RTT_NOM_DIS	(0)
#define DDR3_RTT_NOM_60		(1 << 2)
#define DDR3_RTT_NOM_120	(1 << 6)
#define DDR3_RTT_NOM_40		((1 << 2) | (1 << 6))
#define DDR3_TDQS		(1 << 11)

/* mr2 for ddr3 */
#define DDR3_MR2_CWL(n)		((((n) - 5) & 0x7) << 3)
#define DDR3_RTT_WR_DIS		(0)
#define DDR3_RTT_WR_60		(1 << 9)
#define DDR3_RTT_WR_120		(2 << 9)

/*
 * MR0 (Device Information)
 * 0:DAI complete, 1:DAI still in progress
 */
#define LPDDR2_DAI		(0x1)
/* 0:S2 or S4 SDRAM, 1:NVM */
#define LPDDR2_DI		(0x1 << 1)
/* 0:DNV not supported, 1:DNV supported */
#define LPDDR2_DNVI		(0x1 << 2)
#define LPDDR2_RZQI		(0x3 << 3)

/*
 * 00:RZQ self test not supported,
 * 01:ZQ-pin may connect to VDDCA or float
 * 10:ZQ-pin may short to GND.
 * 11:ZQ-pin self test completed, no error condition detected.
 */

/* MR1 (Device Feature) */
#define LPDDR2_BL4		(0x2)
#define LPDDR2_BL8		(0x3)
#define LPDDR2_BL16		(0x4)
#define LPDDR2_N_WR(n)		(((n) - 2) << 5)

/* MR2 (Device Feature 2) */
#define LPDDR2_RL3_WL1		(0x1)
#define LPDDR2_RL4_WL2		(0x2)
#define LPDDR2_RL5_WL2		(0x3)
#define LPDDR2_RL6_WL3		(0x4)
#define LPDDR2_RL7_WL4		(0x5)
#define LPDDR2_RL8_WL4		(0x6)

/* MR3 (IO Configuration 1) */
#define LPDDR2_DS_34		(0x1)
#define LPDDR2_DS_40		(0x2)
#define LPDDR2_DS_48		(0x3)
#define LPDDR2_DS_60		(0x4)
#define LPDDR2_DS_80		(0x6)
/* optional */
#define LPDDR2_DS_120		(0x7)

/* MR4 (Device Temperature) */
#define LPDDR2_TREF_MASK	(0x7)
#define LPDDR2_4_TREF		(0x1)
#define LPDDR2_2_TREF		(0x2)
#define LPDDR2_1_TREF		(0x3)
#define LPDDR2_025_TREF		(0x5)
#define LPDDR2_025_TREF_DERATE	(0x6)

#define LPDDR2_TUF		(0x1 << 7)

/* MR8 (Basic configuration 4) */
#define LPDDR2_S4		(0x0)
#define LPDDR2_S2		(0x1)
#define LPDDR2_N		(0x2)
/* Unit:MB */
#define LPDDR2_DENSITY(mr8)	(8 << (((mr8) >> 2) & 0xf))
#define LPDDR2_IO_WIDTH(mr8)	(32 >> (((mr8) >> 6) & 0x3))

/* MR10 (Calibration) */
#define LPDDR2_ZQINIT		(0xff)
#define LPDDR2_ZQCL		(0xab)
#define LPDDR2_ZQCS		(0x56)
#define LPDDR2_ZQRESET		(0xc3)

/* MR16 (PASR Bank Mask), S2 SDRAM Only */
#define LPDDR2_PASR_FULL	(0x0)
#define LPDDR2_PASR_1_2		(0x1)
#define LPDDR2_PASR_1_4		(0x2)
#define LPDDR2_PASR_1_8		(0x3)

/*
 * MR0 (Device Information)
 * 0:DAI complete,
 * 1:DAI still in progress
 */
#define LPDDR3_DAI		(0x1)
/*
 * 00:RZQ self test not supported,
 * 01:ZQ-pin may connect to VDDCA or float
 * 10:ZQ-pin may short to GND.
 * 11:ZQ-pin self test completed, no error condition detected.
 */
#define LPDDR3_RZQI		(0x3 << 3)
/*
 * 0:DRAM does not support WL(Set B),
 * 1:DRAM support WL(Set B)
 */
#define LPDDR3_WL_SUPOT		(1 << 6)
/*
 * 0:DRAM does not support RL=3,nWR=3,WL=1;
 * 1:DRAM supports RL=3,nWR=3,WL=1 for frequencies <=166
 */
#define LPDDR3_RL3_SUPOT	(1 << 7)

/* MR1 (Device Feature) */
#define LPDDR3_BL8		(0x3)
#define LPDDR3_N_WR(n)		((n) << 5)

/* MR2 (Device Feature 2), WL Set A,default */
/* <=166MHz,optional*/
#define LPDDR3_RL3_WL1		(0x1)
/* <=400MHz*/
#define LPDDR3_RL6_WL3		(0x4)
/* <=533MHz*/
#define LPDDR3_RL8_WL4		(0x6)
/* <=600MHz*/
#define LPDDR3_RL9_WL5		(0x7)
/* <=667MHz,default*/
#define LPDDR3_RL10_WL6		(0x8)
/* <=733MHz*/
#define LPDDR3_RL11_WL6		(0x9)
/* <=800MHz*/
#define LPDDR3_RL12_WL6		(0xa)
/* <=933MHz*/
#define LPDDR3_RL14_WL8		(0xc)
/* <=1066MHz*/
#define LPDDR3_RL16_WL8		(0xe)

/* WL Set B, optional */
/* <=667MHz,default*/
#define LPDDR3_RL10_WL8		(0x8)
/* <=733MHz*/
#define LPDDR3_RL11_WL9		(0x9)
/* <=800MHz*/
#define LPDDR3_RL12_WL9		(0xa)
/* <=933MHz*/
#define LPDDR3_RL14_WL11	(0xc)
/* <=1066MHz*/
#define LPDDR3_RL16_WL13	(0xe)

/* 1:enable nWR programming > 9(default)*/
#define LPDDR3_N_WRE		(1 << 4)
/* 1:Select WL Set B*/
#define LPDDR3_WL_S		(1 << 6)
/* 1:enable*/
#define LPDDR3_WR_LEVEL		(1 << 7)

/* MR3 (IO Configuration 1) */
#define LPDDR3_DS_34		(0x1)
#define LPDDR3_DS_40		(0x2)
#define LPDDR3_DS_48		(0x3)
#define LPDDR3_DS_60		(0x4)
#define LPDDR3_DS_80		(0x6)
#define LPDDR3_DS_34D_40U	(0x9)
#define LPDDR3_DS_40D_48U	(0xa)
#define LPDDR3_DS_34D_48U	(0xb)

/* MR4 (Device Temperature) */
#define LPDDR3_TREF_MASK	(0x7)
/* SDRAM Low temperature operating limit exceeded */
#define LPDDR3_LT_EXED		(0x0)
#define LPDDR3_4_TREF		(0x1)
#define LPDDR3_2_TREF		(0x2)
#define LPDDR3_1_TREF		(0x3)
#define LPDDR3_05_TREF		(0x4)
#define LPDDR3_025_TREF		(0x5)
#define LPDDR3_025_TREF_DERATE	(0x6)
/* SDRAM High temperature operating limit exceeded */
#define LPDDR3_HT_EXED		(0x7)

/* 1:value has changed since last read of MR4 */
#define LPDDR3_TUF		(0x1 << 7)

/* MR8 (Basic configuration 4) */
#define LPDDR3_S8		(0x3)
#define LPDDR3_DENSITY(mr8)	(8 << (((mr8) >> 2) & 0xf))
#define LPDDR3_IO_WIDTH(mr8)	(32 >> (((mr8) >> 6) & 0x3))

/* MR10 (Calibration) */
#define LPDDR3_ZQINIT		(0xff)
#define LPDDR3_ZQCL		(0xab)
#define LPDDR3_ZQCS		(0x56)
#define LPDDR3_ZQRESET		(0xc3)

/* MR11 (ODT Control) */
#define LPDDR3_ODT_60		(1)
#define LPDDR3_ODT_120		(2)
#define LPDDR3_ODT_240		(3)
#define LPDDR3_ODT_DIS		(0)

/* MR2 (Device Feature 2) */
/* RL & nRTP for DBI-RD Disabled */
#define LPDDR4_RL6_NRTP8	(0x0)
#define LPDDR4_RL10_NRTP8	(0x1)
#define LPDDR4_RL14_NRTP8	(0x2)
#define LPDDR4_RL20_NRTP8	(0x3)
#define LPDDR4_RL24_NRTP10	(0x4)
#define LPDDR4_RL28_NRTP12	(0x5)
#define LPDDR4_RL32_NRTP14	(0x6)
#define LPDDR4_RL36_NRTP16	(0x7)
/* RL & nRTP for DBI-RD Disabled */
#define LPDDR4_RL12_NRTP8	(0x1)
#define LPDDR4_RL16_NRTP8	(0x2)
#define LPDDR4_RL22_NRTP8	(0x3)
#define LPDDR4_RL28_NRTP10	(0x4)
#define LPDDR4_RL32_NRTP12	(0x5)
#define LPDDR4_RL36_NRTP14	(0x6)
#define LPDDR4_RL40_NRTP16	(0x7)
/* WL Set A,default */
#define LPDDR4_A_WL4		(0x0)
#define LPDDR4_A_WL6		(0x1)
#define LPDDR4_A_WL8		(0x2)
#define LPDDR4_A_WL10		(0x3)
#define LPDDR4_A_WL12		(0x4)
#define LPDDR4_A_WL14		(0x5)
#define LPDDR4_A_WL16		(0x6)
#define LPDDR4_A_WL18		(0x7)
/* WL Set B, optional */
#define LPDDR4_B_WL4		(0x0 << 3)
#define LPDDR4_B_WL8		(0x1 << 3)
#define LPDDR4_B_WL12		(0x2 << 3)
#define LPDDR4_B_WL18		(0x3 << 3)
#define LPDDR4_B_WL22		(0x4 << 3)
#define LPDDR4_B_WL26		(0x5 << 3)
#define LPDDR4_B_WL30		(0x6 << 3)
#define LPDDR4_B_WL34		(0x7 << 3)
/* 1:Select WL Set B*/
#define LPDDR4_WL_B		(1 << 6)
/* 1:enable*/
#define LPDDR4_WR_LEVEL		(1 << 7)

/* MR3 */
#define LPDDR4_VDDQ_2_5		(0)
#define LPDDR4_VDDQ_3		(1)
#define LPDDR4_WRPST_0_5_TCK	(0 << 1)
#define LPDDR4_WRPST_1_5_TCK	(1 << 1)
#define LPDDR4_PPR_EN		(1 << 2)
/* PDDS */
#define LPDDR4_PDDS_240		(0x1 << 3)
#define LPDDR4_PDDS_120		(0x2 << 3)
#define LPDDR4_PDDS_80		(0x3 << 3)
#define LPDDR4_PDDS_60		(0x4 << 3)
#define LPDDR4_PDDS_48		(0x5 << 3)
#define LPDDR4_PDDS_40		(0x6 << 3)
#define LPDDR4_DBI_RD_EN	(1 << 6)
#define LPDDR4_DBI_WR_EN	(1 << 7)

/* MR11 (ODT Control) */
#define LPDDR4_DQODT_240	(1)
#define LPDDR4_DQODT_120	(2)
#define LPDDR4_DQODT_80		(3)
#define LPDDR4_DQODT_60		(4)
#define LPDDR4_DQODT_48		(5)
#define LPDDR4_DQODT_40		(6)
#define LPDDR4_DQODT_DIS	(0)
#define LPDDR4_CAODT_240	(1 << 4)
#define LPDDR4_CAODT_120	(2 << 4)
#define LPDDR4_CAODT_80		(3 << 4)
#define LPDDR4_CAODT_60		(4 << 4)
#define LPDDR4_CAODT_48		(5 << 4)
#define LPDDR4_CAODT_40		(6 << 4)
#define LPDDR4_CAODT_DIS	(0 << 4)

/*
 * Description: depend on input parameter "timing_config",
 *		and calculate correspond "dram_type"
 *		spec timing to "pdram_timing"
 * parameters:
 *   input: timing_config
 *   output: pdram_timing
 * NOTE: MR ODT is set, need to disable by controller
 */
void dram_get_parameter(struct timing_related_config *timing_config,
			struct dram_timing_t *pdram_timing);

#endif /* DRAM_SPEC_TIMING_H */
