/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WAKE_H
#define WAKE_H

#include <types.h>
#include <stddef.h>
#include <config.h>
#include <types/short_types.h>

struct sleep_mode;
struct resource_irq;

int32_t wake_finish(void);
void wake_check(void);
void wake_notify_mode(const struct sleep_mode *mode);
const struct sleep_mode *wake_deepest_mode(void);
int32_t wake_init(void);

int32_t wake_sources_assign(const struct resource_irq *irq, size_t n);

void wake_arm(int32_t host);

#endif
