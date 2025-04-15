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

#endif
