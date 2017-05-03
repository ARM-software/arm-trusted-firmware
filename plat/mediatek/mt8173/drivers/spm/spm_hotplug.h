/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SPM_HOTPLUG_H__
#define __SPM_HOTPLUG_H__

void spm_clear_hotplug(void);
void spm_hotplug_off(unsigned long mpidr);
void spm_hotplug_on(unsigned long mpidr);

#endif /* __SPM_HOTPLUG_H__ */
