/*
 * Copyright (c) 2020-2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_RM_H
#define MT_LP_RM_H

#include <stdbool.h>

#define MT_RM_STATUS_OK		(0)
#define MT_RM_STATUS_BAD	(-1)
#define MT_RM_STATUS_STOP	(-2)

enum PLAT_MT_LPM_RC_TYPE {
	PLAT_RC_UPDATE_CONDITION,
	PLAT_RC_STATUS,
	PLAT_RC_UPDATE_REMAIN_IRQS,
	PLAT_RC_IS_FMAUDIO,
	PLAT_RC_IS_ADSP,
	PLAT_RC_ENTER_CNT,
	PLAT_RC_CLKBUF_STATUS,
	PLAT_RC_UFS_STATUS,
	PLAT_RC_IS_USB_PERI,
	PLAT_RC_IS_USB_INFRA,
	PLAT_RC_MAX,
};

enum plat_mt_lpm_hw_ctrl_type {
	PLAT_AP_MDSRC_REQ,
	PLAT_AP_MDSRC_ACK,
	PLAT_AP_IS_MD_SLEEP,
	PLAT_AP_MDSRC_SETTLE,
	PLAT_AP_GPUEB_PLL_CONTROL,
	PLAT_AP_GPUEB_GET_PWR_STATUS,
	PLAT_AP_HW_CTRL_MAX,
};

struct mt_resource_constraint {
	int level;
	int (*init)(void);
	bool (*is_valid)(unsigned int cpu, int stateid);
	int (*update)(int stateid, int type, const void *p);
	int (*run)(unsigned int cpu, int stateid);
	int (*reset)(unsigned int cpu, int stateid);
	int (*get_status)(unsigned int type, void *priv);
	unsigned int (*allow)(int stateid);
};

struct mt_resource_manager {
	int (*update)(struct mt_resource_constraint **con, unsigned int num,
		      int stateid, void *priv);
	struct mt_resource_constraint **consts;
};

extern int mt_lp_rm_register(struct mt_resource_manager *rm);
extern int mt_lp_rm_do_constraint(unsigned int constraint_id, unsigned int cpuid, int stateid);
extern int mt_lp_rm_find_constraint(unsigned int idx, unsigned int cpuid,
				    int stateid, void *priv);
extern int mt_lp_rm_find_and_run_constraint(unsigned int idx, unsigned int cpuid,
					    int stateid, void *priv);
extern int mt_lp_rm_reset_constraint(unsigned int idx, unsigned int cpuid, int stateid);
extern int mt_lp_rm_do_update(int stateid, int type, void const *p);
extern int mt_lp_rm_get_status(unsigned int type, void *priv);

#endif /* MT_LP_RM_H */
