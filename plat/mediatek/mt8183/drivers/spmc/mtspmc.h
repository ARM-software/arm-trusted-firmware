/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_H
#define MTSPMC_H

/*
 * CONFIG_SPMC_MODE: Select CPU power control mode.
 *
 * 0: Legacy
 *   Control power flow from SW through SPM register (MP*_PWR_CON).
 * 1: HW
 *   Control power flow from SPMC. Most control flow and timing are handled
 *   by SPMC.
 */
#define SPMC_MODE   1

int spmc_init(void);

void spm_poweron_cpu(int cluster, int cpu);
void spm_poweroff_cpu(int cluster, int cpu);

void spm_poweroff_cluster(int cluster);
void spm_poweron_cluster(int cluster);

int spm_get_cpu_powerstate(int cluster, int cpu);
int spm_get_cluster_powerstate(int cluster);
int spm_get_powerstate(uint32_t mask);

void spm_enable_cpu_auto_off(int cluster, int cpu);
void spm_disable_cpu_auto_off(int cluster, int cpu);
void spm_set_cpu_power_off(int cluster, int cpu);
void spm_enable_cluster_auto_off(int cluster);

void mcucfg_init_archstate(int cluster, int cpu, int arm64);
void mcucfg_set_bootaddr(int cluster, int cpu, uintptr_t bootaddr);
uintptr_t mcucfg_get_bootaddr(int cluster, int cpu);

#endif /* MTSPMC_H */
