/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef STM32MP_DDRCTRL_REGS_H
#define STM32MP_DDRCTRL_REGS_H

#include <cdefs.h>
#include <stdint.h>

#include <lib/utils_def.h>

/* DDR Controller (DDRCTRL) registers */
struct stm32mp_ddrctl {
	uint32_t mstr ;		/* 0x0 Master */
	uint32_t stat;		/* 0x4 Operating Mode Status */
	uint8_t reserved008[0x10 - 0x8];
	uint32_t mrctrl0;	/* 0x10 Control 0 */
	uint32_t mrctrl1;	/* 0x14 Control 1 */
	uint32_t mrstat;	/* 0x18 Status */
	uint32_t mrctrl2;	/* 0x1c Control 2 */
	uint32_t derateen;	/* 0x20 Temperature Derate Enable */
	uint32_t derateint;	/* 0x24 Temperature Derate Interval */
	uint32_t reserved028;
	uint32_t deratectl;	/* 0x2c Temperature Derate Control */
	uint32_t pwrctl;	/* 0x30 Low Power Control */
	uint32_t pwrtmg;	/* 0x34 Low Power Timing */
	uint32_t hwlpctl;	/* 0x38 Hardware Low Power Control */
	uint8_t reserved03c[0x50 - 0x3c];
	uint32_t rfshctl0;	/* 0x50 Refresh Control 0 */
	uint32_t rfshctl1;	/* 0x54 Refresh Control 1 */
	uint32_t reserved058;	/* 0x58 Refresh Control 2 */
	uint32_t reserved05C;
	uint32_t rfshctl3;	/* 0x60 Refresh Control 0 */
	uint32_t rfshtmg;	/* 0x64 Refresh Timing */
	uint32_t rfshtmg1;	/* 0x68 Refresh Timing 1 */
	uint8_t reserved06c[0xc0 - 0x6c];
	uint32_t crcparctl0;	/* 0xc0 CRC Parity Control0 */
	uint32_t crcparctl1;	/* 0xc4 CRC Parity Control1 */
	uint32_t reserved0c8;	/* 0xc8 CRC Parity Control2 */
	uint32_t crcparstat;	/* 0xcc CRC Parity Status */
	uint32_t init0;		/* 0xd0 SDRAM Initialization 0 */
	uint32_t init1;		/* 0xd4 SDRAM Initialization 1 */
	uint32_t init2;		/* 0xd8 SDRAM Initialization 2 */
	uint32_t init3;		/* 0xdc SDRAM Initialization 3 */
	uint32_t init4;		/* 0xe0 SDRAM Initialization 4 */
	uint32_t init5;		/* 0xe4 SDRAM Initialization 5 */
	uint32_t init6;		/* 0xe8 SDRAM Initialization 6 */
	uint32_t init7;		/* 0xec SDRAM Initialization 7 */
	uint32_t dimmctl;	/* 0xf0 DIMM Control */
	uint32_t rankctl;	/* 0xf4 Rank Control */
	uint8_t reserved0f4[0x100 - 0xf8];
	uint32_t dramtmg0;	/* 0x100 SDRAM Timing 0 */
	uint32_t dramtmg1;	/* 0x104 SDRAM Timing 1 */
	uint32_t dramtmg2;	/* 0x108 SDRAM Timing 2 */
	uint32_t dramtmg3;	/* 0x10c SDRAM Timing 3 */
	uint32_t dramtmg4;	/* 0x110 SDRAM Timing 4 */
	uint32_t dramtmg5;	/* 0x114 SDRAM Timing 5 */
	uint32_t dramtmg6;	/* 0x118 SDRAM Timing 6 */
	uint32_t dramtmg7;	/* 0x11c SDRAM Timing 7 */
	uint32_t dramtmg8;	/* 0x120 SDRAM Timing 8 */
	uint32_t dramtmg9;	/* 0x124 SDRAM Timing 9 */
	uint32_t dramtmg10;	/* 0x128 SDRAM Timing 10 */
	uint32_t dramtmg11;	/* 0x12c SDRAM Timing 11 */
	uint32_t dramtmg12;	/* 0x130 SDRAM Timing 12 */
	uint32_t dramtmg13;	/* 0x134 SDRAM Timing 13 */
	uint32_t dramtmg14;	/* 0x138 SDRAM Timing 14 */
	uint32_t dramtmg15;	/* 0x13c SDRAM Timing 15 */
	uint8_t reserved140[0x180 - 0x140];
	uint32_t zqctl0;	/* 0x180 ZQ Control 0 */
	uint32_t zqctl1;	/* 0x184 ZQ Control 1 */
	uint32_t zqctl2;	/* 0x188 ZQ Control 2 */
	uint32_t zqstat;	/* 0x18c ZQ Status */
	uint32_t dfitmg0;	/* 0x190 DFI Timing 0 */
	uint32_t dfitmg1;	/* 0x194 DFI Timing 1 */
	uint32_t dfilpcfg0;	/* 0x198 DFI Low Power Configuration 0 */
	uint32_t dfilpcfg1;	/* 0x19c DFI Low Power Configuration 1 */
	uint32_t dfiupd0;	/* 0x1a0 DFI Update 0 */
	uint32_t dfiupd1;	/* 0x1a4 DFI Update 1 */
	uint32_t dfiupd2;	/* 0x1a8 DFI Update 2 */
	uint32_t reserved1ac;
	uint32_t dfimisc;	/* 0x1b0 DFI Miscellaneous Control */
	uint32_t dfitmg2;	/* 0x1b4 DFI Timing 2 */
	uint32_t dfitmg3;	/* 0x1b8 DFI Timing 3 */
	uint32_t dfistat;	/* 0x1bc DFI Status */
	uint32_t dbictl;	/* 0x1c0 DM/DBI Control */
	uint32_t dfiphymstr;	/* 0x1c4 DFI PHY Master interface */
	uint8_t reserved1c8[0x200 - 0x1c8];
	uint32_t addrmap0;	/* 0x200 Address Map 0 */
	uint32_t addrmap1;	/* 0x204 Address Map 1 */
	uint32_t addrmap2;	/* 0x208 Address Map 2 */
	uint32_t addrmap3;	/* 0x20c Address Map 3 */
	uint32_t addrmap4;	/* 0x210 Address Map 4 */
	uint32_t addrmap5;	/* 0x214 Address Map 5 */
	uint32_t addrmap6;	/* 0x218 Address Map 6 */
	uint32_t addrmap7;	/* 0x21c Address Map 7 */
	uint32_t addrmap8;	/* 0x220 Address Map 8 */
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
	uint8_t reserved374[0x3f0 - 0x374];
	uint32_t deratestat;	/* 0x3f0 Temperature Derate Status */
	uint8_t reserved3f4[0x3fc - 0x3f4];

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

#if STM32MP_DDR_DUAL_AXI_PORT
	/* PORT 1 */
	uint32_t pcfgr_1;	/* 0x4b4 Configuration Read */
	uint32_t pcfgw_1;	/* 0x4b8 Configuration Write */
	uint8_t reserved4bc[0x540 - 0x4bc];
	uint32_t pctrl_1;	/* 0x540 Port 2 Control Register */
	uint32_t pcfgqos0_1;	/* 0x544 Read QoS Configuration 0 */
	uint32_t pcfgqos1_1;	/* 0x548 Read QoS Configuration 1 */
	uint32_t pcfgwqos0_1;	/* 0x54c Write QoS Configuration 0 */
	uint32_t pcfgwqos1_1;	/* 0x550 Write QoS Configuration 1 */
#endif

	uint8_t reserved554[0xff0 - 0x554];
	uint32_t umctl2_ver_number;	/* 0xff0 UMCTL2 Version Number */
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
#if STM32MP_DDR_DUAL_AXI_PORT
#define DDRCTRL_PCTRL_1				0x540
#endif

/* DDR Controller Register fields */
#define DDRCTRL_MSTR_DDR3			BIT(0)
#define DDRCTRL_MSTR_LPDDR2			BIT(2)
#define DDRCTRL_MSTR_LPDDR3			BIT(3)
#define DDRCTRL_MSTR_DDR4			BIT(4)
#define DDRCTRL_MSTR_LPDDR4			BIT(5)
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

#define DDRCTRL_PWRTMG_SELFREF_TO_X32_MASK	GENMASK(23, 16)
#define DDRCTRL_PWRTMG_SELFREF_TO_X32_0		BIT(16)

#define DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH	BIT(0)
#define DDRCTRL_RFSHCTL3_REFRESH_UPDATE_LEVEL	BIT(1)

#define DDRCTRL_HWLPCTL_HW_LP_EN		BIT(0)

#define DDRCTRL_RFSHTMG_T_RFC_NOM_X1_X32_MASK	GENMASK(27, 16)
#define DDRCTRL_RFSHTMG_T_RFC_NOM_X1_X32_SHIFT	16

#define DDRCTRL_INIT0_SKIP_DRAM_INIT_MASK	GENMASK(31, 30)
#define DDRCTRL_INIT0_SKIP_DRAM_INIT_NORMAL	BIT(30)

#define DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN	BIT(0)
#define DDRCTRL_DFIMISC_DFI_INIT_START		BIT(5)

#define DDRCTRL_DFISTAT_DFI_INIT_COMPLETE	BIT(0)

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

#endif /* STM32MP_DDRCTRL_REGS_H */
