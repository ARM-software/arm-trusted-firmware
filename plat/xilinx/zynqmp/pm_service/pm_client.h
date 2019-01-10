/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
void pm_client_wakeup(const struct pm_proc *proc);
enum pm_ret_status set_ocm_retention(void);
enum pm_ret_status pm_set_suspend_mode(uint32_t mode);
const struct pm_proc *pm_get_proc_by_node(enum pm_node_id nid);

/* Global variables to be set in pm_client.c */
extern const struct pm_proc *primary_proc;

#endif /* PM_CLIENT_H */
