/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/arm/sp805.h>
#include <lib/mmio.h>

/* Inline register access functions */

static inline void sp805_write_wdog_load(uintptr_t base, uint32_t value)
{
	mmio_write_32(base + SP805_WDOG_LOAD_OFF, value);
}

static inline void sp805_write_wdog_ctrl(uintptr_t base, uint32_t value)
{
	mmio_write_32(base + SP805_WDOG_CTR_OFF, value);
}

static inline void sp805_write_wdog_lock(uintptr_t base, uint32_t value)
{
	mmio_write_32(base + SP805_WDOG_LOCK_OFF, value);
}


/* Public API implementation */

void sp805_start(uintptr_t base, unsigned int ticks)
{
	sp805_write_wdog_load(base, ticks);
	sp805_write_wdog_ctrl(base, SP805_CTR_RESEN | SP805_CTR_INTEN);
	/* Lock registers access */
	sp805_write_wdog_lock(base, 0U);
}

void sp805_stop(uintptr_t base)
{
	sp805_write_wdog_lock(base, WDOG_UNLOCK_KEY);
	sp805_write_wdog_ctrl(base, 0U);
}

void sp805_refresh(uintptr_t base, unsigned int ticks)
{
	sp805_write_wdog_lock(base, WDOG_UNLOCK_KEY);
	sp805_write_wdog_load(base, ticks);
	sp805_write_wdog_lock(base, 0U);
}
