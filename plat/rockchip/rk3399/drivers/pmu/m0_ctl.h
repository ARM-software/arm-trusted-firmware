/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __M0_CTL_H__
#define __M0_CTL_H__

#include <m0_param.h>

#define M0_BINCODE_BASE 	((uintptr_t)rk3399m0_bin)
#define M0_PARAM_ADDR		(M0_BINCODE_BASE + PARAM_ADDR)

/* pmu_fw.c */
extern char rk3399m0_bin[];
extern char rk3399m0_bin_end[];

extern void m0_init(void);
extern void m0_start(void);
extern void m0_stop(void);
extern void m0_wait_done(void);
#endif /* __M0_CTL_H__ */
