/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RK3399_MCU_H
#define RK3399_MCU_H

#include <addressmap.h>

typedef unsigned int uint32_t;

#define mmio_read_32(c)	({unsigned int __v = \
				(*(volatile unsigned int *)(c)); __v; })
#define mmio_write_32(c, v)	((*(volatile unsigned int *)(c)) = (v))

#define mmio_clrbits_32(addr, clear) \
		mmio_write_32(addr, (mmio_read_32(addr) & ~(clear)))
#define mmio_setbits_32(addr, set) \
		mmio_write_32(addr, (mmio_read_32(addr)) | (set))
#define mmio_clrsetbits_32(addr, clear, set) \
		mmio_write_32(addr, (mmio_read_32(addr) & ~(clear)) | (set))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void stopwatch_init_usecs_expire(unsigned int usecs);
int stopwatch_expired(void);
void stopwatch_reset(void);

#endif /* RK3399_MCU_H */
