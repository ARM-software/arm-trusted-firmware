/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTSPMC_H__
#define __MTSPMC_H__

#define CONFIG_SPMC_MODE   1    /* 0:Legacy  1:HW  2:SW */

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

#endif /* __MTCMOS_H__ */
