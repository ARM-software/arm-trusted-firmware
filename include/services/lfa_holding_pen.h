/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LFA_HOLDING_PEN_H
#define LFA_HOLDING_PEN_H

#include <stdbool.h>
#include <stdint.h>

#include <services/lfa_svc.h>

bool lfa_holding_start(void);
enum lfa_retc lfa_holding_wait(void);
void lfa_holding_release(enum lfa_retc status);

typedef uint64_t holding_lock_t;
extern holding_lock_t lfa_r_holding_lock_var;
extern uint64_t lfa_r_ep_addresses[PLATFORM_CORE_COUNT];
extern uint64_t lfa_r_context_ids[PLATFORM_CORE_COUNT];

void lfa_load_relocatable(void);
void lfa_r_holding_lock(holding_lock_t *lock);
void lfa_r_holding_unlock(holding_lock_t *lock);
void lfa_r_holding_wait(holding_lock_t *lock);
void lfa_r_holding_wait_warm_reset(holding_lock_t *lock);

#endif
