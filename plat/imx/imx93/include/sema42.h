/*
 * Copyright 2023-2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SEMA42_H
#define SEMA42_H

#include <assert.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <platform_def.h>

static inline int sema42_lock(uint8_t lock_id)
{
	unsigned int timeout = 10000U; /* ~10ms at 1us per iteration */

	assert(lock_id < 64U);

	do {
		mmio_write_8(SEMA42_1_BASE + lock_id, 0x1U);
		if (mmio_read_8(SEMA42_1_BASE + lock_id) == 0x1U) {
			return 0;
		}
		udelay(1);
	} while (--timeout > 0U);

	return -1;
}

static inline void sema42_unlock(uint8_t lock_id)
{
	mmio_write_8(SEMA42_1_BASE + lock_id, 0x0U);
}

#endif /* SEMA42_H */
