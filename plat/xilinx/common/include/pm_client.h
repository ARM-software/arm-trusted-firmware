/*
 * Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
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
void pm_client_suspend(const struct pm_proc *proc, uint32_t state);
void pm_client_abort_suspend(void);
void pm_client_wakeup(const struct pm_proc *proc);

#if !defined(PLAT_zynqmp)
enum pm_device_node_idx irq_to_pm_node_idx(uint32_t irq);
#endif

/* Global variables to be set in pm_client.c */
extern const struct pm_proc *primary_proc;

#if defined(PLAT_zynqmp)
enum pm_ret_status pm_set_suspend_mode(uint32_t mode);
#endif /* PLAT_zynqmp */

#endif /* PM_CLIENT_H */
