/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef WDT_H
#define WDT_H

#include "stdint.h"

void wdt_pet(void);
void wdt_resume(void);
void wdt_set_enable(int enable);
int wdt_set_timeout(uint32_t timeout);
uint64_t wdt_smc_handler(uint32_t x1, uint32_t x2, void *handle);
void wdt_suspend(void);
void wdt_trigger_reset(void);

#endif /* WDT_H */
