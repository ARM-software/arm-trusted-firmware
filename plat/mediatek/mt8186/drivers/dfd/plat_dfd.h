/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
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
#define MISC1_CFG_BASE				(MCU_BIU_BASE + 0xA040)

#define DFD_INTERNAL_CTL			(MISC1_CFG_BASE + 0x00)
#define DFD_INTERNAL_PWR_ON			(MISC1_CFG_BASE + 0x08)
#define DFD_CHAIN_LENGTH0			(MISC1_CFG_BASE + 0x0C)
#define DFD_INTERNAL_SHIFT_CLK_RATIO		(MISC1_CFG_BASE + 0x10)
#define DFD_INTERNAL_TEST_SO_0			(MISC1_CFG_BASE + 0x28)
#define DFD_INTERNAL_NUM_OF_TEST_SO_GROUP	(MISC1_CFG_BASE + 0x30)
#define DFD_V30_CTL				(MISC1_CFG_BASE + 0x48)
#define DFD_V30_BASE_ADDR			(MISC1_CFG_BASE + 0x4C)
#define DFD_TEST_SI_0				(MISC1_CFG_BASE + 0x58)
#define DFD_TEST_SI_1				(MISC1_CFG_BASE + 0x5C)
#define DFD_HW_TRIGGER_MASK			(MISC1_CFG_BASE + 0xBC)

#define DFD_V35_ENALBE				(MCU_BIU_BASE + 0xA0A8)
#define DFD_V35_TAP_NUMBER			(MCU_BIU_BASE + 0xA0AC)
#define DFD_V35_TAP_EN				(MCU_BIU_BASE + 0xA0B0)
#define DFD_V35_SEQ0_0				(MCU_BIU_BASE + 0xA0C0)
#define DFD_V35_SEQ0_1				(MCU_BIU_BASE + 0xA0C4)

#define DFD_CACHE_DUMP_ENABLE			(1U)
#define DFD_PARITY_ERR_TRIGGER			(2U)

#define MCUSYS_DFD_MAP				(0x10001390)
#define WDT_DEBUG_CTL				(0x10007048)

#define WDT_DEBUG_CTL_VAL_0			(0x950603A0)
#define DFD_INTERNAL_TEST_SO_0_VAL		(0x3B)
#define DFD_TEST_SI_0_VAL			(0x108)
#define DFD_TEST_SI_1_VAL			(0x20200000)

#define WDT_DEBUG_CTL_VAL_1			(0x95063E80)
#define DFD_V35_TAP_NUMBER_VAL			(0xA)
#define DFD_V35_TAP_EN_VAL			(0x3FF)
#define DFD_V35_SEQ0_0_VAL			(0x63668820)
#define DFD_HW_TRIGGER_MASK_VAL			(0xC)

void dfd_resume(void);
uint64_t dfd_smc_dispatcher(uint64_t arg0, uint64_t arg1,
			    uint64_t arg2, uint64_t arg3);

#endif /* PLAT_DFD_H */
