/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CAD_WATCHDOG_H__
#define __CAD_WATCHDOG_H__

#define WDT_BASE			(0xFFD00200)
#define WDT_REG_SIZE_OFFSET		(0x4)
#define WDT_MIN_CYCLES			(65536)
#define WDT_PERIOD			(20)

#define WDT_CR				(WDT_BASE + 0x0)
#define WDT_TORR			(WDT_BASE + 0x4)

#define WDT_CRR				(WDT_BASE + 0xC)

#define WDT_CCVR			(WDT_BASE + 0x8)
#define WDT_STAT			(WDT_BASE + 0x10)
#define WDT_EOI				(WDT_BASE + 0x14)

#define WDT_COMP_PARAM_1		(WDT_BASE + 0xF4)
#define WDT_COMP_VERSION		(WDT_BASE + 0xF8)
#define WDT_COMP_TYPE			(WDT_BASE + 0XFC)

#define WDT_CR_RMOD			(0x0)
#define WDT_CR_EN			(0x1)

#define WDT_SW_RST			(0x76)


void watchdog_init(int watchdog_clk);
void watchdog_enable(void);
void watchdog_info(void);
void watchdog_status(void);
void watchdog_sw_rst(void);

#endif
