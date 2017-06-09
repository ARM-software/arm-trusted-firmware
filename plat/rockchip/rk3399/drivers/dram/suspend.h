/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_ROCKCHIP_RK3399_SUSPEND_H__
#define __SOC_ROCKCHIP_RK3399_SUSPEND_H__
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

void dmc_save(void);
__pmusramfunc void dmc_restore(void);

#endif /* __DRAM_H__ */
