/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SPM_SUSPEND_H__
#define __SPM_SUSPEND_H__

/* cpu dormant return code */
#define CPU_DORMANT_RESET        0
#define CPU_DORMANT_ABORT        1

void spm_system_suspend(void);
void spm_system_suspend_finish(void);

#endif /* __SPM_SUSPEND_H__*/
