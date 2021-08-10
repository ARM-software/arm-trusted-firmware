/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DFD_H
#define PLAT_DFD_H

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <platform_def.h>

#define sync_writel(addr, val)	do { mmio_write_32((addr), (val)); \
				dsbsy(); \
				} while (0)

#define PLAT_MTK_DFD_SETUP_MAGIC		(0x99716150)
#define PLAT_MTK_DFD_READ_MAGIC			(0x99716151)
#define PLAT_MTK_DFD_WRITE_MAGIC		(0x99716152)

#define MCU_BIU_BASE				(MCUCFG_BASE)
#define MISC1_CFG_BASE				(MCU_BIU_BASE + 0xE040)
#define DFD_INTERNAL_CTL			(MISC1_CFG_BASE + 0x00)
#define DFD_INTERNAL_PWR_ON			(MISC1_CFG_BASE + 0x08)
#define DFD_CHAIN_LENGTH0			(MISC1_CFG_BASE + 0x0C)
#define DFD_INTERNAL_SHIFT_CLK_RATIO		(MISC1_CFG_BASE + 0x10)
#define DFD_CHAIN_LENGTH1			(MISC1_CFG_BASE + 0x1C)
#define DFD_CHAIN_LENGTH2			(MISC1_CFG_BASE + 0x20)
#define DFD_CHAIN_LENGTH3			(MISC1_CFG_BASE + 0x24)
#define DFD_INTERNAL_TEST_SO_0			(MISC1_CFG_BASE + 0x28)
#define DFD_INTERNAL_NUM_OF_TEST_SO_GROUP	(MISC1_CFG_BASE + 0x30)
#define DFD_INTERNAL_TEST_SO_OVER_64		(MISC1_CFG_BASE + 0x34)
#define DFD_V30_CTL				(MISC1_CFG_BASE + 0x48)
#define DFD_V30_BASE_ADDR			(MISC1_CFG_BASE + 0x4C)
#define DFD_POWER_CTL				(MISC1_CFG_BASE + 0x50)
#define DFD_TEST_SI_0				(MISC1_CFG_BASE + 0x58)
#define DFD_TEST_SI_1				(MISC1_CFG_BASE + 0x5C)
#define DFD_CLEAN_STATUS			(MISC1_CFG_BASE + 0x60)
#define DFD_TEST_SI_2				(MISC1_CFG_BASE + 0x1D8)
#define DFD_TEST_SI_3				(MISC1_CFG_BASE + 0x1DC)
#define DFD_HW_TRIGGER_MASK			(MISC1_CFG_BASE + 0xBC)

#define DFD_V35_ENALBE				(MCU_BIU_BASE + 0xE0A8)
#define DFD_V35_TAP_NUMBER			(MCU_BIU_BASE + 0xE0AC)
#define DFD_V35_TAP_EN				(MCU_BIU_BASE + 0xE0B0)
#define DFD_V35_CTL				(MCU_BIU_BASE + 0xE0B4)
#define DFD_V35_SEQ0_0				(MCU_BIU_BASE + 0xE0C0)
#define DFD_V35_SEQ0_1				(MCU_BIU_BASE + 0xE0C4)

#define DFD_O_PROTECT_EN_REG			(0x10001220)
#define DFD_O_INTRF_MCU_PWR_CTL_MASK		(0x10001A3C)
#define DFD_O_SET_BASEADDR_REG			(0x10043034)

#define DFD_CACHE_DUMP_ENABLE			1U
#define DFD_PARITY_ERR_TRIGGER			2U

#define DFD_TEST_SI_0_CACHE_DIS_VAL		(0x1E000202)
#define DFD_TEST_SI_0_CACHE_EN_VAL		(0x1E000002)
#define DFD_TEST_SI_1_VAL			(0x20408100)
#define DFD_TEST_SI_2_VAL			(0x10101000)
#define DFD_TEST_SI_3_VAL			(0x00000010)
#define	DFD_V35_TAP_EN_VAL			(0x43FF)
#define	DFD_V35_SEQ0_0_VAL			(0x63668820)

void dfd_resume(void);
uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
			    uint64_t arg2, uint64_t arg3);

#endif /* PLAT_DFD_H */
