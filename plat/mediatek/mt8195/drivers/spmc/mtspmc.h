/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_H
#define MTSPMC_H

#include <stdint.h>

int spmc_init(void);

void spm_poweron_cpu(unsigned int cluster, unsigned int cpu);
void spm_poweroff_cpu(unsigned int cluster, unsigned int cpu);

void spm_poweroff_cluster(unsigned int cluster);
void spm_poweron_cluster(unsigned int cluster);

bool spm_get_cpu_powerstate(unsigned int cluster, unsigned int cpu);
bool spm_get_cluster_powerstate(unsigned int cluster);
bool spm_get_powerstate(uint32_t mask);

void mcucfg_init_archstate(unsigned int cluster, unsigned int cpu, bool arm64);
void mcucfg_set_bootaddr(unsigned int cluster, unsigned int cpu, uintptr_t bootaddr);
uintptr_t mcucfg_get_bootaddr(unsigned int cluster, unsigned int cpu);

void mcucfg_disable_gic_wakeup(unsigned int cluster, unsigned int cpu);
void mcucfg_enable_gic_wakeup(unsigned int cluster, unsigned int cpu);

#endif /* MTSPMC_H */
