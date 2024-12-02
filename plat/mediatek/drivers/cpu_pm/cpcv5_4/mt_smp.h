/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SMP_H
#define MT_SMP_H

#include <lib/mmio.h>
#include <platform_def.h>

#include "mt_cpu_pm.h"

#define CPUSYS_PPU_CLK_EN_CTRL		BIT(12)
#define CPUSYS_PPU_ISO_CTRL		BIT(13)
#define AA64NAA32_FLAG_START_BIT	16

#define SMP_CORE_TIMEOUT_MAX		(50000)
#define DO_SMP_CORE_ON_WAIT_TIMEOUT(cpu_id, k_cnt) ({ \
	if (k_cnt >= SMP_CORE_TIMEOUT_MAX) { \
		INFO("[%s:%d] - CORE%d ON WAIT TIMEOUT %u us (> %u)\n", \
		     __func__, __LINE__, cpu_id, k_cnt, SMP_CORE_TIMEOUT_MAX); \
		panic(); \
	} \
	k_cnt++; udelay(1); })

#ifdef CPU_PM_CORE_ARCH64_ONLY
#define mt_smp_core_init_arch(_a, _b, _c, _d)
#else
void mt_smp_core_init_arch(int cluster, int cpu, int arm64,
			   struct cpu_pwr_ctrl *pwr_ctrl);
#endif /* CPU_PM_CORE_ARCH64_ONLY */

void mt_smp_core_bootup_address_set(int cluster,
				    int cpu,
				    struct cpu_pwr_ctrl *pwr_ctrl,
				    uintptr_t entry);

int mt_smp_power_core_on(unsigned int cpu_id, struct cpu_pwr_ctrl *pwr_ctrl);
int mt_smp_power_core_off(unsigned int cpu_id, struct cpu_pwr_ctrl *pwr_ctrl);

void mt_smp_init(void);

int mt_smp_cluster_pwpr_init(struct cluster_pwr_ctrl *pwr_ctrl);
int mt_smp_cluster_pwpr_op_init(struct cluster_pwr_ctrl *pwr_ctrl);

#endif /* MT_SMP_H */
