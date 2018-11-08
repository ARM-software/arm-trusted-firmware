/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_RK3368_H
#define DDR_RK3368_H

#define DDR_PCTL_SCFG		0x0
#define DDR_PCTL_SCTL		0x4
#define DDR_PCTL_STAT		0x8
#define DDR_PCTL_INTRSTAT	0xc

#define DDR_PCTL_MCMD		0x40
#define DDR_PCTL_POWCTL		0x44
#define DDR_PCTL_POWSTAT	0x48
#define DDR_PCTL_CMDTSTAT	0x4c
#define DDR_PCTL_CMDTSTATEN	0x50
#define DDR_PCTL_MRRCFG0	0x60
#define DDR_PCTL_MRRSTAT0	0x64
#define DDR_PCTL_MRRSTAT1	0x68
#define DDR_PCTL_MCFG1		0x7c
#define DDR_PCTL_MCFG		0x80
#define DDR_PCTL_PPCFG		0x84
#define DDR_PCTL_MSTAT		0x88
#define DDR_PCTL_LPDDR2ZQCFG	0x8c
#define DDR_PCTL_DTUPDES		0x94
#define DDR_PCTL_DTUNA			0x98
#define DDR_PCTL_DTUNE			0x9c
#define DDR_PCTL_DTUPRD0		0xa0
#define DDR_PCTL_DTUPRD1		0xa4
#define DDR_PCTL_DTUPRD2		0xa8
#define DDR_PCTL_DTUPRD3		0xac
#define DDR_PCTL_DTUAWDT		0xb0
#define DDR_PCTL_TOGCNT1U		0xc0
#define DDR_PCTL_TINIT			0xc4
#define DDR_PCTL_TRSTH			0xc8
#define DDR_PCTL_TOGCNT100N		0xcc
#define DDR_PCTL_TREFI			0xd0
#define DDR_PCTL_TMRD			0xd4
#define DDR_PCTL_TRFC			0xd8
#define DDR_PCTL_TRP			0xdc
#define DDR_PCTL_TRTW			0xe0
#define DDR_PCTL_TAL			0xe4
#define DDR_PCTL_TCL			0xe8
#define DDR_PCTL_TCWL			0xec
#define DDR_PCTL_TRAS			0xf0
#define DDR_PCTL_TRC			0xf4
#define DDR_PCTL_TRCD			0xf8
#define DDR_PCTL_TRRD			0xfc
#define DDR_PCTL_TRTP			0x100
#define DDR_PCTL_TWR			0x104
#define DDR_PCTL_TWTR			0x108
#define DDR_PCTL_TEXSR			0x10c
#define DDR_PCTL_TXP			0x110
#define DDR_PCTL_TXPDLL			0x114
#define DDR_PCTL_TZQCS			0x118
#define DDR_PCTL_TZQCSI			0x11c
#define DDR_PCTL_TDQS			0x120
#define DDR_PCTL_TCKSRE			0x124
#define DDR_PCTL_TCKSRX			0x128
#define DDR_PCTL_TCKE			0x12c
#define DDR_PCTL_TMOD			0x130
#define DDR_PCTL_TRSTL			0x134
#define DDR_PCTL_TZQCL			0x138
#define DDR_PCTL_TMRR			0x13c
#define DDR_PCTL_TCKESR			0x140
#define DDR_PCTL_TDPD			0x144
#define DDR_PCTL_TREFI_MEM_DDR3	0x148
#define DDR_PCTL_ECCCFG			0x180
#define DDR_PCTL_ECCTST			0x184
#define DDR_PCTL_ECCCLR			0x188
#define DDR_PCTL_ECCLOG			0x18c
#define DDR_PCTL_DTUWACTL		0x200
#define DDR_PCTL_DTURACTL		0x204
#define DDR_PCTL_DTUCFG			0x208
#define DDR_PCTL_DTUECTL		0x20c
#define DDR_PCTL_DTUWD0			0x210
#define DDR_PCTL_DTUWD1			0x214
#define DDR_PCTL_DTUWD2			0x218
#define DDR_PCTL_DTUWD3			0x21c
#define DDR_PCTL_DTUWDM			0x220
#define DDR_PCTL_DTURD0			0x224
#define DDR_PCTL_DTURD1			0x228
#define DDR_PCTL_DTURD2			0x22c
#define DDR_PCTL_DTURD3			0x230
#define DDR_PCTL_DTULFSRWD		0x234
#define DDR_PCTL_DTULFSRRD		0x238
#define DDR_PCTL_DTUEAF			0x23c
#define DDR_PCTL_DFITCTRLDELAY	0x240
#define DDR_PCTL_DFIODTCFG		0x244
#define DDR_PCTL_DFIODTCFG1		0x248
#define DDR_PCTL_DFIODTRANKMAP		0x24c
#define DDR_PCTL_DFITPHYWRDATA		0x250
#define DDR_PCTL_DFITPHYWRLAT		0x254
#define DDR_PCTL_DFITPHYWRDATALAT	0x258
#define DDR_PCTL_DFITRDDATAEN		0x260
#define DDR_PCTL_DFITPHYRDLAT		0x264
#define DDR_PCTL_DFITPHYUPDTYPE0	0x270
#define DDR_PCTL_DFITPHYUPDTYPE1	0x274
#define DDR_PCTL_DFITPHYUPDTYPE2	0x278
#define DDR_PCTL_DFITPHYUPDTYPE3	0x27c
#define DDR_PCTL_DFITCTRLUPDMIN		0x280
#define DDR_PCTL_DFITCTRLUPDMAX		0x284
#define DDR_PCTL_DFITCTRLUPDDLY		0x288
#define DDR_PCTL_DFIUPDCFG			0x290
#define DDR_PCTL_DFITREFMSKI		0x294
#define DDR_PCTL_DFITCTRLUPDI		0x298
#define DDR_PCTL_DFITRCFG0			0x2ac
#define DDR_PCTL_DFITRSTAT0			0x2b0
#define DDR_PCTL_DFITRWRLVLEN		0x2b4
#define DDR_PCTL_DFITRRDLVLEN		0x2b8
#define DDR_PCTL_DFITRRDLVLGATEEN	0x2bc
#define DDR_PCTL_DFISTSTAT0			0x2c0
#define DDR_PCTL_DFISTCFG0			0x2c4
#define DDR_PCTL_DFISTCFG1			0x2c8
#define DDR_PCTL_DFITDRAMCLKEN		0x2d0
#define DDR_PCTL_DFITDRAMCLKDIS		0x2d4
#define DDR_PCTL_DFISTCFG2			0x2d8
#define DDR_PCTL_DFISTPARCLR		0x2dc
#define DDR_PCTL_DFISTPARLOG		0x2e0
#define DDR_PCTL_DFILPCFG0			0x2f0
#define DDR_PCTL_DFITRWRLVLRESP0	0x300
#define DDR_PCTL_DFITRWRLVLRESP1	0x304
#define DDR_PCTL_DFITRWRLVLRESP2	0x308
#define DDR_PCTL_DFITRRDLVLRESP0	0x30c
#define DDR_PCTL_DFITRRDLVLRESP1	0x310
#define DDR_PCTL_DFITRRDLVLRESP2	0x314
#define DDR_PCTL_DFITRWRLVLDELAY0	0x318
#define DDR_PCTL_DFITRWRLVLDELAY1	0x31c
#define DDR_PCTL_DFITRWRLVLDELAY2	0x320
#define DDR_PCTL_DFITRRDLVLDELAY0	0x324
#define DDR_PCTL_DFITRRDLVLDELAY1	0x328
#define DDR_PCTL_DFITRRDLVLDELAY2	0x32c
#define DDR_PCTL_DFITRRDLVLGATEDELAY0	0x330
#define DDR_PCTL_DFITRRDLVLGATEDELAY1	0x334
#define DDR_PCTL_DFITRRDLVLGATEDELAY2	0x338
#define DDR_PCTL_DFITRCMD			0x33c
#define DDR_PCTL_IPVR				0x3f8
#define DDR_PCTL_IPTR				0x3fc

/* DDR PHY REG */
#define DDR_PHY_REG0		0x0
#define DDR_PHY_REG1		0x4
#define DDR_PHY_REG2		0x8
#define DDR_PHY_REG3		0xc
#define DDR_PHY_REG4		0x10
#define DDR_PHY_REG5		0x14
#define DDR_PHY_REG6		0x18
#define DDR_PHY_REGB		0x2c
#define DDR_PHY_REGC		0x30
#define DDR_PHY_REG11		0x44
#define DDR_PHY_REG12		0x48
#define DDR_PHY_REG13		0x4c
#define DDR_PHY_REG14		0x50
#define DDR_PHY_REG16		0x58
#define DDR_PHY_REG20		0x80
#define DDR_PHY_REG21		0x84
#define DDR_PHY_REG26		0x98
#define DDR_PHY_REG27		0x9c
#define DDR_PHY_REG28		0xa0
#define DDR_PHY_REG2C		0xb0
#define DDR_PHY_REG30		0xc0
#define DDR_PHY_REG31		0xc4
#define DDR_PHY_REG36		0xd8
#define DDR_PHY_REG37		0xdc
#define DDR_PHY_REG38		0xe0
#define DDR_PHY_REG3C		0xf0
#define DDR_PHY_REG40		0x100
#define DDR_PHY_REG41		0x104
#define DDR_PHY_REG46		0x118
#define DDR_PHY_REG47		0x11c
#define DDR_PHY_REG48		0x120
#define DDR_PHY_REG4C		0x130
#define DDR_PHY_REG50		0x140
#define DDR_PHY_REG51		0x144
#define DDR_PHY_REG56		0x158
#define DDR_PHY_REG57		0x15c
#define DDR_PHY_REG58		0x160
#define DDR_PHY_REG5C		0x170
#define DDR_PHY_REGDLL		0x290
#define DDR_PHY_REGEC		0x3b0
#define DDR_PHY_REGED		0x3b4
#define DDR_PHY_REGEE		0x3b8
#define DDR_PHY_REGEF		0x3bc
#define DDR_PHY_REGF0		0x3c0
#define DDR_PHY_REGF1		0x3c4
#define DDR_PHY_REGF2		0x3c8
#define DDR_PHY_REGFA		0x3e8
#define DDR_PHY_REGFB		0x3ec
#define DDR_PHY_REGFC		0x3f0
#define DDR_PHY_REGFD		0x3f4
#define DDR_PHY_REGFE		0x3f8
#define DDR_PHY_REGFF		0x3fc

/* MSCH REG define */
#define MSCH_COREID			0x0
#define MSCH_DDRCONF		0x8
#define MSCH_DDRTIMING		0xc
#define MSCH_DDRMODE		0x10
#define MSCH_READLATENCY	0x14
#define MSCH_ACTIVATE		0x38
#define MSCH_DEVTODEV		0x3c

#define SET_NR(n)      ((0x3f << (8 + 16)) | ((n - 1) << 8))
#define SET_NO(n)      ((0xf << (0 + 16)) | ((n - 1) << 0))
#define SET_NF(n)      ((n - 1) & 0x1fff)
#define SET_NB(n)      ((n - 1) & 0xfff)
#define PLLMODE(n)     ((0x3 << (8 + 16)) | (n << 8))

/* GRF REG define */
#define GRF_SOC_STATUS0		0x480
#define GRF_DDRPHY_LOCK		(0x1 << 15)
#define GRF_DDRC0_CON0		0x600

/* CRU softreset ddr pctl, phy */
#define DDRMSCH0_SRSTN_REQ(n)  (((0x1 << 10) << 16) | (n << 10))
#define DDRCTRL0_PSRSTN_REQ(n) (((0x1 << 3) << 16) | (n << 3))
#define DDRCTRL0_SRSTN_REQ(n)  (((0x1 << 2) << 16) | (n << 2))
#define DDRPHY0_PSRSTN_REQ(n)  (((0x1 << 1) << 16) | (n << 1))
#define DDRPHY0_SRSTN_REQ(n)   (((0x1 << 0) << 16) | (n << 0))

/* CRU_DPLL_CON2 */
#define DPLL_STATUS_LOCK		(1 << 31)

/* CRU_DPLL_CON3 */
#define DPLL_POWER_DOWN			((0x1 << (1 + 16)) | (0 << 1))
#define DPLL_WORK_NORMAL_MODE		((0x3 << (8 + 16)) | (0 << 8))
#define DPLL_WORK_SLOW_MODE		((0x3 << (8 + 16)) | (1 << 8))
#define DPLL_RESET_CONTROL_NORMAL	((0x1 << (5 + 16)) | (0x0 << 5))
#define DPLL_RESET_CONTROL_RESET	((0x1 << (5 + 16)) | (0x1 << 5))

/* PMU_PWRDN_CON */
#define PD_PERI_PWRDN_ENABLE		(1 << 13)

#define DDR_PLL_SRC_MASK		0x13

/* DDR_PCTL_TREFI */
#define DDR_UPD_REF_ENABLE		(0X1 << 31)

uint32_t ddr_get_resume_code_size(void);
uint32_t ddr_get_resume_data_size(void);
uint32_t *ddr_get_resume_code_base(void);
void ddr_reg_save(uint32_t pllpdstat, uint64_t base_addr);

#endif /* DDR_RK3368_H */
