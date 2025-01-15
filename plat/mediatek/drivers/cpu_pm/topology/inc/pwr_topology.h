/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWR_TOPOLOGY_H
#define PWR_TOPOLOGY_H

#include <lib/pm/mtk_pm.h>

enum pwr_domain_status {
	PWR_DOMAIN_ON,
	PWR_DOMAIN_OFF,
	PWR_DOMAIN_SMP_ON,
	PWR_DOMAIN_SMP_OFF,
};

struct pwr_toplogy {
	unsigned int cur_group_bit;
	unsigned int group;
};

typedef int (*afflv_prepare)(unsigned int,
			     const struct mtk_cpupm_pwrstate *,
			     const struct pwr_toplogy *);

void pwr_topology_init(void);

unsigned int pwr_domain_coordination(enum pwr_domain_status pwr,
				     const mtk_pstate_type psci_state,
				     const struct mtk_cpupm_pwrstate *state,
				     afflv_prepare fn);

#endif
