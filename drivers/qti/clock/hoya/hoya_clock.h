/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Clock init sequences shared by the hoya SoCs. Each chipset calls the ones
 * for the blocks it has.
 */

#ifndef HOYA_CLOCK_H
#define HOYA_CLOCK_H

#include <stddef.h>
#include <stdint.h>

void hoya_clock_tz_gdsc_init(uintptr_t sgdscr);
void hoya_clock_gdsc_retain(uintptr_t gdsc_ctl);
void hoya_clock_retention_votes(const uintptr_t *votes, size_t num);

/* hoya_clock_gpu.c, for the chipsets that build it. */
int hoya_clock_gpu_init(void);
void hoya_clock_gpu_gdsc_disable(void);

/* hoya_clock_lpass.c, for the chipsets that boot from LPASS memory. */
void hoya_clock_lpass_imem_on(void);
void hoya_clock_lpass_q6_reset(void);

#endif /* HOYA_CLOCK_H */
