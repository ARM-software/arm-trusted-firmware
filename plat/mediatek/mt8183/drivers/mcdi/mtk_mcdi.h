/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_MCDI_H__
#define __MTK_MCDI_H__

#include <stdbool.h>

void sspm_set_bootaddr(uint32_t bootaddr);
void sspm_standbywfi_irq_enable(uint32_t cpu_idx);
void sspm_cluster_pwr_off_notify(uint32_t cluster);
void sspm_cluster_pwr_on_notify(uint32_t cluster);

uint32_t mcdi_avail_cpu_mask_read(void);
uint32_t mcdi_avail_cpu_mask_write(uint32_t mask);
uint32_t mcdi_avail_cpu_mask_set(uint32_t mask);
uint32_t mcdi_avail_cpu_mask_clr(uint32_t mask);
uint32_t mcdi_cpu_cluster_pwr_stat_read(void);

void mcdi_pause(void);
void mcdi_unpause(void);
void mcdi_pause_set(int cluster, int cpu_idx, bool on);
void mcdi_pause_clr(int cluster, int cpu_idx, bool on);
void mcdi_hotplug_set(int cluster, int cpu_idx, bool on);
void mcdi_hotplug_clr(int cluster, int cpu_idx, bool on);
void mcdi_hotplug_wait_ack(int cluster, int cpu_idx, bool on);

bool check_mcdi_ctl_stat(void);
void mcdi_init(void);

#endif /* __MTK_MCDI_H__ */
