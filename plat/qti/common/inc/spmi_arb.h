/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_ARB_H
#define SPMI_ARB_H

#include <stdint.h>

/*******************************************************************************
 * WARNING: This driver does not arbitrate access with the kernel. These APIs
 * must only be called when the kernel is known to be quiesced (such as before
 * boot or while the system is shutting down).
 ******************************************************************************/

/* 32-bit addresses combine (U)SID, PID and register address. */

int spmi_arb_read8(uint32_t addr);
int spmi_arb_write8(uint32_t addr, uint8_t data);

#endif /* SPMI_ARB_H */
