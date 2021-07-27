/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUSPEND_H
#define SUSPEND_H

#include <stdint.h>
#include <dram.h>

#define KHz (1000)
#define MHz (1000 * KHz)
#define GHz (1000 * MHz)

#define PI_CA_TRAINING		(1 << 0)
#define PI_WRITE_LEVELING	(1 << 1)
#define PI_READ_GATE_TRAINING	(1 << 2)
#define PI_READ_LEVELING	(1 << 3)
#define PI_WDQ_LEVELING		(1 << 4)
#define PI_FULL_TRAINING	(0xff)

void dmc_suspend(void);
__pmusramfunc void dmc_resume(void);
extern __pmusramdata uint8_t pmu_enable_watchdog0;

#endif /* SUSPEND_H */
