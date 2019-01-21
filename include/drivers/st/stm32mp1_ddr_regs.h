/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef STM32MP1_DDR_REGS_H
#define STM32MP1_DDR_REGS_H

#include <lib/utils_def.h>

/* DDR3/LPDDR2/LPDDR3 Controller (DDRCTRL) registers */
struct stm32mp1_ddrctl {
	uint32_t mstr ;		/* 0x0 Master */
	uint32_t stat;		/* 0x4 Operating Mode Status */
	uint8_t reserved008[0x10 - 0x8];
	uint32_t mrctrl0;	/* 0x10 Control 0 */
	uint32_t mrctrl1;	/* 0x14 Control 1 */
	uint32_t mrstat;	/* 0x18 Status */
	uint32_t reserved01c;	/* 0x1c */
	uint32_t derateen;	/* 0x20 Temperature Derate Enable */
	uint32_t derateint;	/* 0x24 Temperature Derate Interval */
	uint8_t reserved028[0x30 - 0x28];
	uint32_t pwrctl;	/* 0x30 Low Power Control */
	uint32_t pwrtmg;	/* 0x34 Low Power Timing */
	uint32_t hwlpctl;	/* 0x38 Hardware Low Power Control */
	uint8_t reserved03c[0x50 - 0x3C];
	uint32_t rfshctl0;	/* 0x50 Refresh Control 0 */
	uint32_t reserved054;	/* 0x54 Refresh Control 1 */
	uint32_t reserved058;	/* 0x58 Refresh Control 2 */
	uint32_t reserved05C;
	uint32_t rfshctl3;	/* 0x60 Refresh Control 0 */
	uint32_t rfshtmg;	/* 0x64 Refresh Timing */
	uint8_t reserved068[0xc0 - 0x68];
	uint32_t crcparctl0;		/* 0xc0 CRC Parity Control0 */
	uint32_t reserved0c4;	/* 0xc4 CRC Parity Control1 */
	uint32_t reserved0c8;	/* 0xc8 CRC Parity Control2 */
	uint32_t crcparstat;		/* 0xcc CRC Parity Status */
	uint32_t init0;		/* 0xd0 SDRAM Initialization 0 */
	uint32_t init1;		/* 0xd4 SDRAM Initialization 1 */
	uint32_t init2;		/* 0xd8 SDRAM Initialization 2 */
	uint32_t init3;		/* 0xdc SDRAM Initialization 3 */
	uint32_t init4;		/* 0xe0 SDRAM Initialization 4 */
	uint32_t init5;		/* 0xe4 SDRAM Initialization 5 */
	uint32_t reserved0e8;
	uint32_t reserved0ec;
	uint32_t dimmctl;	/* 0xf0 DIMM Control */
	uint8_t reserved0f4[0x100 - 0xf4];
	uint32_t dramtmg0;	/* 0x100 SDRAM Timing 0 */
	uint32_t dramtmg1;	/* 0x104 SDRAM Timing 1 */
	uint32_t dramtmg2;	/* 0x108 SDRAM Timing 2 */
	uint32_t dramtmg3;	/* 0x10c SDRAM Timing 3 */
	uint32_t dramtmg4;	/* 0x110 SDRAM Timing 4 */
	uint32_t dramtmg5;	/* 0x114 SDRAM Timing 5 */
	uint32_t dramtmg6;	/* 0x118 SDRAM Timing 6 */
	uint32_t dramtmg7;	/* 0x11c SDRAM Timing 7 */
	uint32_t dramtmg8;	/* 0x120 SDRAM Timing 8 */
	uint8_t reserved124[0x138 - 0x124];
	uint32_t dramtmg14;	/* 0x138 SDRAM Timing 14 */
	uint32_t dramtmg15;	/* 0x13C SDRAM Timing 15 */
	uint8_t reserved140[0x180 - 0x140];
	uint32_t zqctl0;	/* 0x180 ZQ Control 0 */
	uint32_t zqctl1;	/* 0x184 ZQ Control 1 */
	uint32_t zqctl2;	/* 0x188 ZQ Control 2 */
	uint32_t zqstat;	/* 0x18c ZQ Status */
	uint32_t dfitmg0;	/* 0x190 DFI Timing 0 */
	uint32_t dfitmg1;	/* 0x194 DFI Timing 1 */
	uint32_t dfilpcfg0;	/* 0x198 DFI Low Power Configuration 0 */
	uint32_t reserved19c;
	uint32_t dfiupd0;	/* 0x1a0 DFI Update 0 */
	uint32_t dfiupd1;	/* 0x1a4 DFI Update 1 */
	uint32_t dfiupd2;	/* 0x1a8 DFI Update 2 */
	uint32_t reserved1ac;
	uint32_t dfimisc;	/* 0x1b0 DFI Miscellaneous Control */
	uint8_t reserved1b4[0x1bc - 0x1b4];
	uint32_t dfistat;	/* 0x1bc DFI Miscellaneous Control */
	uint8_t reserved1c0[0x1c4 - 0x1c0];
	uint32_t dfiphymstr;	/* 0x1c4 DFI PHY Master interface */
	uint8_t reserved1c8[0x204 - 0x1c8];
	uint32_t addrmap1;	/* 0x204 Address Map 1 */
	uint32_t addrmap2;	/* 0x208 Address Map 2 */
	uint32_t addrmap3;	/* 0x20c Address Map 3 */
	uint32_t addrmap4;	/* 0x210 Address Map 4 */
	uint32_t addrmap5;	/* 0x214 Address Map 5 */
	uint32_t addrmap6;	/* 0x218 Address Map 6 */
	uint8_t reserved21c[0x224 - 0x21c];
	uint32_t addrmap9;	/* 0x224 Address Map 9 */
	uint32_t addrmap10;	/* 0x228 Address Map 10 */
	uint32_t addrmap11;	/* 0x22C Address Map 11 */
	uint8_t reserved230[0x240 - 0x230];
	uint32_t odtcfg;	/* 0x240 ODT Configuration */
	uint32_t odtmap;	/* 0x244 ODT/Rank Map */
	uint8_t reserved248[0x250 - 0x248];
	uint32_t sched;		/* 0x250 Scheduler Control */
	uint32_t sched1;	/* 0x254 Scheduler Control 1 */
	uint32_t reserved258;
	uint32_t perfhpr1;	/* 0x25c High Priority Read CAM 1 */
	uint32_t reserved260;
	uint32_t perflpr1;	/* 0x264 Low Priority Read CAM 1 */
	uint32_t reserved268;
	uint32_t perfwr1;	/* 0x26c Write CAM 1 */
	uint8_t reserved27c[0x300 - 0x270];
	uint32_t dbg0;		/* 0x300 Debug 0 */
	uint32_t dbg1;		/* 0x304 Debug 1 */
	uint32_t dbgcam;	/* 0x308 CAM Debug */
	uint32_t dbgcmd;	/* 0x30c Command Debug */
	uint32_t dbgstat;	/* 0x310 Status Debug */
	uint8_t reserved314[0x320 - 0x314];
	uint32_t swctl;		/* 0x320 Software Programming Control Enable */
	uint32_t swstat;	/* 0x324 Software Programming Control Status */
	uint8_t reserved328[0x36c - 0x328];
	uint32_t poisoncfg;	/* 0x36c AXI Poison Configuration Register */
	uint32_t poisonstat;	/* 0x370 AXI Poison Status Register */
	uint8_t reserved374[0x3fc - 0x374];

	/* Multi Port registers */
	uint32_t pstat;		/* 0x3fc Port Status */
	uint32_t pccfg;		/* 0x400 Port Common Configuration */

	/* PORT 0 */
	uint32_t pcfgr_0;	/* 0x404 Configuration Read */
	uint32_t pcfgw_0;	/* 0x408 Configuration Write */
	uint8_t reserved40c[0x490 - 0x40c];
	uint32_t pctrl_0;	/* 0x490 Port Control Register */
	uint32_t pcfgqos0_0;	/* 0x494 Read QoS Configuration 0 */
	uint32_t pcfgqos1_0;	/* 0x498 Read QoS Configuration 1 */
	uint32_t pcfgwqos0_0;	/* 0x49c Write QoS Configuration 0 */
	uint32_t pcfgwqos1_0;	/* 0x4a0 Write QoS Configuration 1 */
	uint8_t reserved4a4[0x4b4 - 0x4a4];

	/* PORT 1 */
	uint32_t pcfgr_1;	/* 0x4b4 Configuration Read */
	uint32_t pcfgw_1;	/* 0x4b8 Configuration Write */
	uint8_t reserved4bc[0x540 - 0x4bc];
	uint32_t pctrl_1;	/* 0x540 Port 2 Control Register */
	uint32_t pcfgqos0_1;	/* 0x544 Read QoS Configuration 0 */
	uint32_t pcfgqos1_1;	/* 0x548 Read QoS Configuration 1 */
	uint32_t pcfgwqos0_1;	/* 0x54c Write QoS Configuration 0 */
	uint32_t pcfgwqos1_1;	/* 0x550 Write QoS Configuration 1 */
} __packed;

/* DDR Physical Interface Control (DDRPHYC) registers*/
struct stm32mp1_ddrphy {
	uint32_t ridr;		/* 0x00 R Revision Identification */
	uint32_t pir;		/* 0x04 R/W PHY Initialization */
	uint32_t pgcr;		/* 0x08 R/W PHY General Configuration */
	uint32_t pgsr;		/* 0x0C PHY General Status */
	uint32_t dllgcr;	/* 0x10 R/W DLL General Control */
	uint32_t acdllcr;	/* 0x14 R/W AC DLL Control */
	uint32_t ptr0;		/* 0x18 R/W PHY Timing 0 */
	uint32_t ptr1;		/* 0x1C R/W PHY Timing 1 */
	uint32_t ptr2;		/* 0x20 R/W PHY Timing 2 */
	uint32_t aciocr;	/* 0x24 AC I/O Configuration */
	uint32_t dxccr;		/* 0x28 DATX8 Common Configuration */
	uint32_t dsgcr;		/* 0x2C DDR System General Configuration */
	uint32_t dcr;		/* 0x30 DRAM Configuration */
	uint32_t dtpr0;		/* 0x34 DRAM Timing Parameters0 */
	uint32_t dtpr1;		/* 0x38 DRAM Timing Parameters1 */
	uint32_t dtpr2;		/* 0x3C DRAM Timing Parameters2 */
	uint32_t mr0;		/* 0x40 Mode 0 */
	uint32_t mr1;		/* 0x44 Mode 1 */
	uint32_t mr2;		/* 0x48 Mode 2 */
	uint32_t mr3;		/* 0x4C Mode 3 */
	uint32_t odtcr;		/* 0x50 ODT Configuration */
	uint32_t dtar;		/* 0x54 data training address */
	uint32_t dtdr0;		/* 0x58 */
	uint32_t dtdr1;		/* 0x5c */
	uint8_t res1[0x0c0 - 0x060];	/* 0x60 */
	uint32_t dcuar;		/* 0xc0 Address */
	uint32_t dcudr;		/* 0xc4 DCU Data */
	uint32_t dcurr;		/* 0xc8 DCU Run */
	uint32_t dculr;		/* 0xcc DCU Loop */
	uint32_t dcugcr;	/* 0xd0 DCU General Configuration */
	uint32_t dcutpr;	/* 0xd4 DCU Timing Parameters */
	uint32_t dcusr0;	/* 0xd8 DCU Status 0 */
	uint32_t dcusr1;	/* 0xdc DCU Status 1 */
	uint8_t res2[0x100 - 0xe0];	/* 0xe0 */
	uint32_t bistrr;	/* 0x100 BIST Run */
	uint32_t bistmskr0;	/* 0x104 BIST Mask 0 */
	uint32_t bistmskr1;	/* 0x108 BIST Mask 0 */
	uint32_t bistwcr;	/* 0x10c BIST Word Count */
	uint32_t bistlsr;	/* 0x110 BIST LFSR Seed */
	uint32_t bistar0;	/* 0x114 BIST Address 0 */
	uint32_t bistar1;	/* 0x118 BIST Address 1 */
	uint32_t bistar2;	/* 0x11c BIST Address 2 */
	uint32_t bistupdr;	/* 0x120 BIST User Data Pattern */
	uint32_t bistgsr;	/* 0x124 BIST General Status */
	uint32_t bistwer;	/* 0x128 BIST Word Error */
	uint32_t bistber0;	/* 0x12c BIST Bit Error 0 */
	uint32_t bistber1;	/* 0x130 BIST Bit Error 1 */
	uint32_t bistber2;	/* 0x134 BIST Bit Error 2 */
	uint32_t bistwcsr;	/* 0x138 BIST Word Count Status */
	uint32_t bistfwr0;	/* 0x13c BIST Fail Word 0 */
	uint32_t bistfwr1;	/* 0x140 BIST Fail Word 1 */
	uint8_t res3[0x178 - 0x144];	/* 0x144 */
	uint32_t gpr0;		/* 0x178 General Purpose 0 (GPR0) */
	uint32_t gpr1;		/* 0x17C General Purpose 1 (GPR1) */
	uint32_t zq0cr0;	/* 0x180 zq 0 control 0 */
	uint32_t zq0cr1;	/* 0x184 zq 0 control 1 */
	uint32_t zq0sr0;	/* 0x188 zq 0 status 0 */
	uint32_t zq0sr1;	/* 0x18C zq 0 status 1 */
	uint8_t res4[0x1C0 - 0x190];	/* 0x190 */
	uint32_t dx0gcr;	/* 0x1c0 Byte lane 0 General Configuration */
	uint32_t dx0gsr0;	/* 0x1c4 Byte lane 0 General Status 0 */
	uint32_t dx0gsr1;	/* 0x1c8 Byte lane 0 General Status 1 */
	uint32_t dx0dllcr;	/* 0x1cc Byte lane 0 DLL Control */
	uint32_t dx0dqtr;	/* 0x1d0 Byte lane 0 DQ Timing */
	uint32_t dx0dqstr;	/* 0x1d4 Byte lane 0 DQS Timing */
	uint8_t res5[0x200 - 0x1d8];	/* 0x1d8 */
	uint32_t dx1gcr;	/* 0x200 Byte lane 1 General Configuration */
	uint32_t dx1gsr0;	/* 0x204 Byte lane 1 General Status 0 */
	uint32_t dx1gsr1;	/* 0x208 Byte lane 1 General Status 1 */
	uint32_t dx1dllcr;	/* 0x20c Byte lane 1 DLL Control */
	uint32_t dx1dqtr;	/* 0x210 Byte lane 1 DQ Timing */
	uint32_t dx1dqstr;	/* 0x214 Byte lane 1 QS Timing */
	uint8_t res6[0x240 - 0x218];	/* 0x218 */
	uint32_t dx2gcr;	/* 0x240 Byte lane 2 General Configuration */
	uint32_t dx2gsr0;	/* 0x244 Byte lane 2 General Status 0 */
	uint32_t dx2gsr1;	/* 0x248 Byte lane 2 General Status 1 */
	uint32_t dx2dllcr;	/* 0x24c Byte lane 2 DLL Control */
	uint32_t dx2dqtr;	/* 0x250 Byte lane 2 DQ Timing */
	uint32_t dx2dqstr;	/* 0x254 Byte lane 2 QS Timing */
	uint8_t res7[0x280 - 0x258];	/* 0x258 */
	uint32_t dx3gcr;	/* 0x280 Byte lane 3 General Configuration */
	uint32_t dx3gsr0;	/* 0x284 Byte lane 3 General Status 0 */
	uint32_t dx3gsr1;	/* 0x288 Byte lane 3 General Status 1 */
	uint32_t dx3dllcr;	/* 0x28c Byte lane 3 DLL Control */
	uint32_t dx3dqtr;	/* 0x290 Byte lane 3 DQ Timing */
	uint32_t dx3dqstr;	/* 0x294 Byte lane 3 QS Timing */
} __packed;

/* DDR Controller registers offsets */
#define DDRCTRL_MSTR				0x000
#define DDRCTRL_STAT				0x004
#define DDRCTRL_MRCTRL0				0x010
#define DDRCTRL_MRSTAT				0x018
#define DDRCTRL_PWRCTL				0x030
#define DDRCTRL_PWRTMG				0x034
#define DDRCTRL_HWLPCTL				0x038
#define DDRCTRL_RFSHCTL3			0x060
#define DDRCTRL_RFSHTMG				0x064
#define DDRCTRL_INIT0				0x0D0
#define DDRCTRL_DFIMISC				0x1B0
#define DDRCTRL_DBG1				0x304
#define DDRCTRL_DBGCAM				0x308
#define DDRCTRL_DBGCMD				0x30C
#define DDRCTRL_DBGSTAT				0x310
#define DDRCTRL_SWCTL				0x320
#define DDRCTRL_SWSTAT				0x324
#define DDRCTRL_PSTAT				0x3FC
#define DDRCTRL_PCTRL_0				0x490
#define DDRCTRL_PCTRL_1				0x540

/* DDR Controller Register fields */
#define DDRCTRL_MSTR_DDR3			BIT(0)
#define DDRCTRL_MSTR_LPDDR2			BIT(2)
#define DDRCTRL_MSTR_LPDDR3			BIT(3)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_MASK	GENMASK(13, 12)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_FULL	0
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_HALF	BIT(12)
#define DDRCTRL_MSTR_DATA_BUS_WIDTH_QUARTER	BIT(13)
#define DDRCTRL_MSTR_DLL_OFF_MODE		BIT(15)

#define DDRCTRL_STAT_OPERATING_MODE_MASK	GENMASK(2, 0)
#define DDRCTRL_STAT_OPERATING_MODE_NORMAL	BIT(0)
#define DDRCTRL_STAT_OPERATING_MODE_SR		(BIT(0) | BIT(1))
#define DDRCTRL_STAT_SELFREF_TYPE_MASK		GENMASK(5, 4)
#define DDRCTRL_STAT_SELFREF_TYPE_ASR		(BIT(4) | BIT(5))
#define DDRCTRL_STAT_SELFREF_TYPE_SR		BIT(5)

#define DDRCTRL_MRCTRL0_MR_TYPE_WRITE		U(0)
/* Only one rank supported */
#define DDRCTRL_MRCTRL0_MR_RANK_SHIFT		4
#define DDRCTRL_MRCTRL0_MR_RANK_ALL \
					BIT(DDRCTRL_MRCTRL0_MR_RANK_SHIFT)
#define DDRCTRL_MRCTRL0_MR_ADDR_SHIFT		12
#define DDRCTRL_MRCTRL0_MR_ADDR_MASK		GENMASK(15, 12)
#define DDRCTRL_MRCTRL0_MR_WR			BIT(31)

#define DDRCTRL_MRSTAT_MR_WR_BUSY		BIT(0)

#define DDRCTRL_PWRCTL_SELFREF_EN		BIT(0)
#define DDRCTRL_PWRCTL_POWERDOWN_EN		BIT(1)
#define DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE	BIT(3)
#define DDRCTRL_PWRCTL_SELFREF_SW		BIT(5)

#define DDRCTRL_PWRTMG_SELFREF_TO_X32_MASK	GENMASK(19, 12)
#define DDRCTRL_PWRTMG_SELFREF_TO_X32_0		BIT(16)

#define DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH	BIT(0)

#define DDRCTRL_HWLPCTL_HW_LP_EN		BIT(0)

#define DDRCTRL_RFSHTMG_T_RFC_NOM_X1_X32_MASK	GENMASK(27, 16)
#define DDRCTRL_RFSHTMG_T_RFC_NOM_X1_X32_SHIFT	16

#define DDRCTRL_INIT0_SKIP_DRAM_INIT_MASK	GENMASK(31, 30)
#define DDRCTRL_INIT0_SKIP_DRAM_INIT_NORMAL	BIT(30)

#define DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN	BIT(0)

#define DDRCTRL_DBG1_DIS_HIF			BIT(1)

#define DDRCTRL_DBGCAM_WR_DATA_PIPELINE_EMPTY	BIT(29)
#define DDRCTRL_DBGCAM_RD_DATA_PIPELINE_EMPTY	BIT(28)
#define DDRCTRL_DBGCAM_DBG_WR_Q_EMPTY		BIT(26)
#define DDRCTRL_DBGCAM_DBG_LPR_Q_DEPTH		GENMASK(12, 8)
#define DDRCTRL_DBGCAM_DBG_HPR_Q_DEPTH		GENMASK(4, 0)
#define DDRCTRL_DBGCAM_DATA_PIPELINE_EMPTY \
		(DDRCTRL_DBGCAM_WR_DATA_PIPELINE_EMPTY | \
		 DDRCTRL_DBGCAM_RD_DATA_PIPELINE_EMPTY)
#define DDRCTRL_DBGCAM_DBG_Q_DEPTH \
		(DDRCTRL_DBGCAM_DBG_WR_Q_EMPTY | \
		 DDRCTRL_DBGCAM_DBG_LPR_Q_DEPTH | \
		 DDRCTRL_DBGCAM_DBG_HPR_Q_DEPTH)

#define DDRCTRL_DBGCMD_RANK0_REFRESH		BIT(0)

#define DDRCTRL_DBGSTAT_RANK0_REFRESH_BUSY	BIT(0)

#define DDRCTRL_SWCTL_SW_DONE			BIT(0)

#define DDRCTRL_SWSTAT_SW_DONE_ACK		BIT(0)

#define DDRCTRL_PCTRL_N_PORT_EN			BIT(0)

/* DDR PHY registers offsets */
#define DDRPHYC_PIR				0x004
#define DDRPHYC_PGCR				0x008
#define DDRPHYC_PGSR				0x00C
#define DDRPHYC_DLLGCR				0x010
#define DDRPHYC_ACDLLCR				0x014
#define DDRPHYC_PTR0				0x018
#define DDRPHYC_ACIOCR				0x024
#define DDRPHYC_DXCCR				0x028
#define DDRPHYC_DSGCR				0x02C
#define DDRPHYC_ZQ0CR0				0x180
#define DDRPHYC_DX0GCR				0x1C0
#define DDRPHYC_DX0DLLCR			0x1CC
#define DDRPHYC_DX1GCR				0x200
#define DDRPHYC_DX1DLLCR			0x20C
#define DDRPHYC_DX2GCR				0x240
#define DDRPHYC_DX2DLLCR			0x24C
#define DDRPHYC_DX3GCR				0x280
#define DDRPHYC_DX3DLLCR			0x28C

/* DDR PHY Register fields */
#define DDRPHYC_PIR_INIT			BIT(0)
#define DDRPHYC_PIR_DLLSRST			BIT(1)
#define DDRPHYC_PIR_DLLLOCK			BIT(2)
#define DDRPHYC_PIR_ZCAL			BIT(3)
#define DDRPHYC_PIR_ITMSRST			BIT(4)
#define DDRPHYC_PIR_DRAMRST			BIT(5)
#define DDRPHYC_PIR_DRAMINIT			BIT(6)
#define DDRPHYC_PIR_QSTRN			BIT(7)
#define DDRPHYC_PIR_ICPC			BIT(16)
#define DDRPHYC_PIR_ZCALBYP			BIT(30)
#define DDRPHYC_PIR_INITSTEPS_MASK		GENMASK(31, 7)

#define DDRPHYC_PGCR_DFTCMP			BIT(2)
#define DDRPHYC_PGCR_PDDISDX			BIT(24)
#define DDRPHYC_PGCR_RFSHDT_MASK		GENMASK(28, 25)

#define DDRPHYC_PGSR_IDONE			BIT(0)
#define DDRPHYC_PGSR_DTERR			BIT(5)
#define DDRPHYC_PGSR_DTIERR			BIT(6)
#define DDRPHYC_PGSR_DFTERR			BIT(7)
#define DDRPHYC_PGSR_RVERR			BIT(8)
#define DDRPHYC_PGSR_RVEIRR			BIT(9)

#define DDRPHYC_DLLGCR_BPS200			BIT(23)

#define DDRPHYC_ACDLLCR_DLLSRST			BIT(30)
#define DDRPHYC_ACDLLCR_DLLDIS			BIT(31)

#define DDRPHYC_PTR0_TDLLSRST_OFFSET		0
#define DDRPHYC_PTR0_TDLLSRST_MASK		GENMASK(5, 0)
#define DDRPHYC_PTR0_TDLLLOCK_OFFSET		6
#define DDRPHYC_PTR0_TDLLLOCK_MASK		GENMASK(17, 6)
#define DDRPHYC_PTR0_TITMSRST_OFFSET		18
#define DDRPHYC_PTR0_TITMSRST_MASK		GENMASK(21, 18)

#define DDRPHYC_ACIOCR_ACPDD			BIT(3)
#define DDRPHYC_ACIOCR_ACPDR			BIT(4)
#define DDRPHYC_ACIOCR_CKPDD_MASK		GENMASK(10, 8)
#define DDRPHYC_ACIOCR_CKPDD_0			BIT(8)
#define DDRPHYC_ACIOCR_CKPDR_MASK		GENMASK(13, 11)
#define DDRPHYC_ACIOCR_CKPDR_0			BIT(11)
#define DDRPHYC_ACIOCR_CSPDD_MASK		GENMASK(21, 18)
#define DDRPHYC_ACIOCR_CSPDD_0			BIT(18)
#define DDRPHYC_ACIOCR_RSTPDD			BIT(27)
#define DDRPHYC_ACIOCR_RSTPDR			BIT(28)

#define DDRPHYC_DXCCR_DXPDD			BIT(2)
#define DDRPHYC_DXCCR_DXPDR			BIT(3)

#define DDRPHYC_DSGCR_CKEPDD_MASK		GENMASK(19, 16)
#define DDRPHYC_DSGCR_CKEPDD_0			BIT(16)
#define DDRPHYC_DSGCR_ODTPDD_MASK		GENMASK(23, 20)
#define DDRPHYC_DSGCR_ODTPDD_0			BIT(20)
#define DDRPHYC_DSGCR_NL2PD			BIT(24)

#define DDRPHYC_ZQ0CRN_ZDATA_MASK		GENMASK(27, 0)
#define DDRPHYC_ZQ0CRN_ZDATA_SHIFT		0
#define DDRPHYC_ZQ0CRN_ZDEN			BIT(28)
#define DDRPHYC_ZQ0CRN_ZQPD			BIT(31)

#define DDRPHYC_DXNGCR_DXEN			BIT(0)

#define DDRPHYC_DXNDLLCR_DLLSRST		BIT(30)
#define DDRPHYC_DXNDLLCR_DLLDIS			BIT(31)
#define DDRPHYC_DXNDLLCR_SDPHASE_MASK		GENMASK(17, 14)
#define DDRPHYC_DXNDLLCR_SDPHASE_SHIFT		14

#endif /* STM32MP1_DDR_REGS_H */
