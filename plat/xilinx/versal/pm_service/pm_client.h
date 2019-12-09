/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains APU specific macros and macros to be defined depending on
 * the execution environment.
 */

#ifndef PM_CLIENT_H
#define PM_CLIENT_H

#include "pm_common.h"
#include "pm_defs.h"

/* Functions to be implemented by each PU */
void pm_client_suspend(const struct pm_proc *proc, unsigned int state);
void pm_client_abort_suspend(void);

/* Global variables to be set in pm_client.c */
extern const struct pm_proc *primary_proc;

#endif /* PM_CLIENT_H */
