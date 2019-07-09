/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SP805_H
#define SP805_H

#include <lib/utils_def.h>

/* SP805 register offset */
#define SP805_WDOG_LOAD_OFF		UL(0x000)
#define SP805_WDOG_CTR_OFF		UL(0x008)
#define SP805_WDOG_LOCK_OFF		UL(0xc00)

/* Magic word to unlock the wd registers */
#define WDOG_UNLOCK_KEY			U(0x1ACCE551)

/* Register field definitions */
#define SP805_CTR_RESEN			(U(1) << 1)
#define SP805_CTR_INTEN			(U(1) << 0)

#ifndef __ASSEMBLER__

#include <stdint.h>

/* Public high level API */

void sp805_start(uintptr_t base, unsigned int ticks);
void sp805_stop(uintptr_t base);
void sp805_refresh(uintptr_t base, unsigned int ticks);

#endif /* __ASSEMBLER__ */

#endif /* SP805_H */
