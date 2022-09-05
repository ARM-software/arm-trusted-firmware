/*
 * Copyright (c) 2020-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_RM_H
#define MT_LP_RM_H

#include <stdbool.h>

#define MT_RM_STATUS_OK		0
#define MT_RM_STATUS_BAD	-1

enum PLAT_MT_LPM_RC_TYPE {
	PLAT_RC_UPDATE_CONDITION,
	PLAT_RC_UPDATE_REMAIN_IRQS
};

struct mt_resource_constraint {
	int level;
	int (*init)(void);
	bool (*is_valid)(unsigned int cpu, int stateid);
	int (*update)(int stateid, int type, const void *p);
	int (*run)(unsigned int cpu, int stateid);
	int (*reset)(unsigned int cpu, int stateid);
	unsigned int (*allow)(int stateid);
};

struct mt_resource_manager {
	int (*update)(struct mt_resource_constraint **con,
		      int stateid, void *priv);
	struct mt_resource_constraint **consts;
};

extern int mt_lp_rm_register(struct mt_resource_manager *rm);
extern int mt_lp_rm_find_and_run_constraint(int idx, unsigned int cpuid,
					    int stateid, void *priv);
extern int mt_lp_rm_reset_constraint(int constraint_id, unsigned int cpuid,
				     int stateid);
extern int mt_lp_rm_do_update(int stateid, int type, void const *p);
#endif /* MT_LP_RM_H */
