/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SP805_H__
#define __SP805_H__

/* SP805 register offset */
#define SP805_WDOG_LOAD_OFF		0x000
#define SP805_WDOG_CTR_OFF		0x008
#define SP805_WDOG_LOCK_OFF		0xc00

/* Magic word to unlock the wd registers */
#define WDOG_UNLOCK_KEY			0x1ACCE551

/* Register field definitions */
#define SP805_CTR_RESEN			(1 << 1)
#define SP805_CTR_INTEN			(1 << 0)

#ifndef __ASSEMBLY__

#include <stdint.h>

/* Public high level API */

void sp805_start(uintptr_t base, unsigned long ticks);
void sp805_stop(uintptr_t base);
void sp805_refresh(uintptr_t base, unsigned long ticks);

#endif /* __ASSEMBLY__ */

#endif /* __SP805_H__ */
