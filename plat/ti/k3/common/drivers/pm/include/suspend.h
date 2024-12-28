/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUSPEND_H
#define SUSPEND_H

int32_t suspend(void);
int32_t suspend_enter(void);
int32_t arch_suspend(void);
int32_t arch_resume(void);

#endif
