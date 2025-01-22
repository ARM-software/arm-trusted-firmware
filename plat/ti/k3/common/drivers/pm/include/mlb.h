/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MLB_H
#define MLB_H

#define MLB_REV				0x000
#define MLB_PWR				0x004
#define MLB_PERF			0x100

#define MLB_PWR_MSTANDBY		BIT(0)

#define MLB_PERF_WMP			BIT(16)
#define MLB_PERF_ASYNC_PRI(N)		((N) << 12)
#define MLB_PERF_ASYNC_PRI_MASK		(7 << 12)
#define MLB_PERF_SYNC_PRI(N)		((N) << 8)
#define MLB_PERF_SYNC_PRI_MASK		(7 << 8)
#define MLB_PERF_ASYNC_FLAG_LOW		0
#define MLB_PERF_ASYNC_FLAG_MED		(1 << 4)
#define MLB_PERF_ASYNC_FLAG_HIGH	(3 << 4)
#define MLB_PERF_ASYNC_FLAG_MASK	(3 << 4)
#define MLB_PERF_SYNC_FLAG_LOW		0
#define MLB_PERF_SYNC_FLAG_MED		1
#define MLB_PERF_SYNC_FLAG_HIGH		3
#define MLB_PERF_SYNC_FLAG_MASK		3

struct drv;

extern const struct drv mlb_drv;

#endif
