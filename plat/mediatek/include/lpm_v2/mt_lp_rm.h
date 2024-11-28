/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_RM_H
#define MT_LP_RM_H

#include <stdbool.h>

#define MT_RM_STATUS_OK		0
#define MT_RM_STATUS_BAD	-1
#define MT_RM_STATUS_STOP	-2

enum plat_mt_lpm_rc_type {
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
	PLAT_RC_IS_USB_HEADSET,
	PLAT_RC_MAX,
};

enum plat_mt_lpm_hw_ctrl_type {
	PLAT_AP_MDSRC_REQ,
	PLAT_AP_MDSRC_ACK,
	PLAT_AP_IS_MD_SLEEP,
	PLAT_AP_MDSRC_SETTLE,
	PLAT_AP_GPUEB_PLL_CONTROL,
	PLAT_AP_GPUEB_PWR_STATUS,
	PLAT_AP_GPUEB_MFG0_PWR_CON,
	PLAT_AP_ASSERT_SPM_IRQ,
	PLAT_AP_SPM_RESOURCE_REQUEST_UPDATE,
	PLAT_AP_SPM_WDT_TRIGGER,
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
	int (*hwctrl)(unsigned int type, int set, void *priv);
};

extern int mt_lp_rm_register(struct mt_resource_manager *rm);
extern int mt_lp_rm_do_constraint(unsigned int constraint_id,
				  unsigned int cpuid, int stateid);
extern int mt_lp_rm_find_constraint(unsigned int idx, unsigned int cpuid,
				    int stateid, void *priv);
extern int mt_lp_rm_find_and_run_constraint(unsigned int idx,
					    unsigned int cpuid,
					    int stateid, void *priv);
extern int mt_lp_rm_reset_constraint(unsigned int idx,
				     unsigned int cpuid, int stateid);
extern int mt_lp_rm_do_update(int stateid, int type, void const *p);
extern int mt_lp_rm_get_status(unsigned int type, void *priv);
extern int mt_lp_rm_do_hwctrl(unsigned int type, int set, void *priv);

#endif /* MT_LP_RM_H */
