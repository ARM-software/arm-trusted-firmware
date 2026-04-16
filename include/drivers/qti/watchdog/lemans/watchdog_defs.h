/*
 * Copyright (c) 2026 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WATCHDOG_DEFS_H
#define WATCHDOG_DEFS_H

#include "lemans_def.h"

#define WDOG_BARK_INT_ID		(0x204)

#define WDOG_REG_BASE			(QTI_AOSS_BASE + 0x01230000)
#define WDOG_RESET_ADDR			(WDOG_REG_BASE + 0x0)
#define RESET				0x1
#define WDOG_CTL_ADDR			(WDOG_REG_BASE + 0x4)
#define ENABLE_BIT			BIT(0)
#define CLK_ENABLE_BIT			BIT(31)
#define HW_SLEEP_WAKEUP_EN		0x2
#define CHIP_AUTOPET_EN			0x4
#define WDOG_BARK_ADDR			(WDOG_REG_BASE + 0xc)
#define WDOG_BARK_MASK			0xfffff
#define BARK_SYNC_BIT			BIT(31)
#define WDOG_BITE_ADDR			(WDOG_REG_BASE + 0x10)
#define WDOG_BITE_MASK			0xfffff
#define BITE_SYNC_BIT			BIT(31)

#endif /* WATCHDOG_DEFS_H */
