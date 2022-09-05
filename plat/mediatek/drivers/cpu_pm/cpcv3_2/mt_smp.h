/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SMP_H
#define MT_SMP_H

#include <lib/mmio.h>
#include <platform_def.h>

#define CPU_PWR_STATUS			(MCUCFG_BASE + 0xA840)

#define SMP_CORE_TIMEOUT_MAX		(50000)
#define DO_SMP_CORE_ON_WAIT_TIMEOUT(k_cnt) ({ \
		CPU_PM_ASSERT(k_cnt < SMP_CORE_TIMEOUT_MAX); \
		k_cnt++; udelay(1); })

void mt_smp_core_init_arch(unsigned int cluster, unsigned int cpu, int arm64,
			   struct cpu_pwr_ctrl *pwr_ctrl);
void mt_smp_core_bootup_address_set(struct cpu_pwr_ctrl *pwr_ctrl, uintptr_t entry);
int mt_smp_power_core_on(unsigned int cpu_id, struct cpu_pwr_ctrl *pwr_ctrl);
int mt_smp_power_core_off(struct cpu_pwr_ctrl *pwr_ctrl);
void mt_smp_init(void);

#endif /* MT_SMP_H */
