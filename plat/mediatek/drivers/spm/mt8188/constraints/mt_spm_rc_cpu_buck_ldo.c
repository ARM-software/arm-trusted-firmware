/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/spm/mt_spm_resource_req.h>
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lpm_smc.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_notifier.h>
#include "mt_spm_rc_api.h"
#include "mt_spm_rc_internal.h"
#include <mt_spm_reg.h>
#include <mt_spm_suspend.h>

#define CONSTRAINT_CPU_BUCK_PCM_FLAG (SPM_FLAG_DISABLE_INFRA_PDN | \
				      SPM_FLAG_DISABLE_VCORE_DVS | \
				      SPM_FLAG_DISABLE_VCORE_DFS | \
				      SPM_FLAG_SRAM_SLEEP_CTRL | \
				      SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP | \
				      SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_CPU_BUCK_PCM_FLAG1 (0)

#define CONSTRAINT_CPU_BUCK_RESOURCE_REQ (MT_SPM_DRAM_S1 | \
					  MT_SPM_DRAM_S0 | \
					  MT_SPM_SYSPLL | \
					  MT_SPM_INFRA | \
					  MT_SPM_26M | \
					  MT_SPM_XO_FPM)

static unsigned int cpubuckldo_status = (MT_SPM_RC_VALID_SW | MT_SPM_RC_VALID_TRACE_TIME);
static unsigned int cpubuckldo_enter_cnt;

int spm_cpu_bcuk_ldo_conduct(int state_id,
			     struct spm_lp_scen *spm_lp,
			     unsigned int *resource_req)
{
	unsigned int res_req = CONSTRAINT_CPU_BUCK_RESOURCE_REQ;

	if ((spm_lp == NULL) || (resource_req == NULL)) {
		return -1;
	}

	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_CPU_BUCK_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_CPU_BUCK_PCM_FLAG1;

	*resource_req |= res_req;
	return 0;
}

bool spm_is_valid_rc_cpu_buck_ldo(unsigned int cpu, int state_id)
{
	return IS_MT_RM_RC_READY(cpubuckldo_status);
}

static void update_rc_status(const void *val)
{
	const struct rc_common_state *st = (const struct rc_common_state *)val;

	if (st == NULL) {
		return;
	}

	if ((st->type == CONSTRAINT_UPDATE_VALID) && st->value) {
		if ((st->id == MT_RM_CONSTRAINT_ID_ALL) ||
		    (st->id == MT_RM_CONSTRAINT_ID_CPU_BUCK_LDO)) {
			struct constraint_status *con = (struct constraint_status *)st->value;

			if ((st->act & MT_LPM_SMC_ACT_CLR) > 0U) {
				SPM_RC_BITS_CLR(cpubuckldo_status, con->is_valid);
			} else {
				SPM_RC_BITS_SET(cpubuckldo_status, con->is_valid);
			}
		}
	}
}

int spm_update_rc_cpu_buck_ldo(int state_id, int type, const void *val)
{
	if (type == PLAT_RC_STATUS) {
		update_rc_status(val);
	}
	return MT_RM_STATUS_OK;
}

unsigned int spm_allow_rc_cpu_buck_ldo(int state_id)
{
	return MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF;
}

int spm_run_rc_cpu_buck_ldo(unsigned int cpu, int state_id)
{
	(void)cpu;
	unsigned int ext_op = 0U;

#ifndef MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER,
			       (IS_PLAT_SUSPEND_ID(state_id) ?
				MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND : (0U)));
#endif
	if (cpubuckldo_status & MT_SPM_RC_VALID_TRACE_TIME) {
		ext_op |= MT_SPM_EX_OP_TRACE_TIMESTAMP_EN;
	}

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_CLR_26M_RECORD |
				      MT_SPM_EX_OP_SET_SUSPEND_MODE |
				      MT_SPM_EX_OP_SET_WDT),
				     CONSTRAINT_CPU_BUCK_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op, spm_cpu_bcuk_ldo_conduct);
	}

	cpubuckldo_enter_cnt++;

	return 0;
}

int spm_reset_rc_cpu_buck_ldo(unsigned int cpu, int state_id)
{
	(void)cpu;
	unsigned int ext_op = 0U;

#ifndef MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, 0U);
#endif
	if (cpubuckldo_status & MT_SPM_RC_VALID_TRACE_TIME) {
		ext_op |= MT_SPM_EX_OP_TRACE_TIMESTAMP_EN;
	}

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_resume(state_id, MT_SPM_EX_OP_SET_WDT, NULL);
	} else {
		mt_spm_idle_generic_resume(state_id, ext_op, NULL, NULL);
	}

	return 0;
}

int spm_get_status_rc_cpu_buck_ldo(unsigned int type, void *priv)
{
	int ret = MT_RM_STATUS_OK;

	if (type != PLAT_RC_STATUS) {
		return ret;
	}

	struct rc_common_state *st = (struct rc_common_state *)priv;

	if (st == NULL) {
		return MT_RM_STATUS_BAD;
	}

	if ((st->id == MT_RM_CONSTRAINT_ID_ALL) ||
	    (st->id == MT_RM_CONSTRAINT_ID_CPU_BUCK_LDO)) {
		struct constraint_status *dest;

		dest = (struct constraint_status *)st->value;
		do {
			if (dest == NULL) {
				break;
			}
			if (st->type == CONSTRAINT_GET_VALID) {
				dest->is_valid = cpubuckldo_status;
			} else if (st->type == CONSTRAINT_COND_BLOCK) {
				dest->is_cond_block = 0;
			} else if (st->type == CONSTRAINT_GET_ENTER_CNT) {
				if (st->id == MT_RM_CONSTRAINT_ID_ALL) {
					dest->enter_cnt += cpubuckldo_enter_cnt;
				} else {
					dest->enter_cnt = cpubuckldo_enter_cnt;
				}
			} else {
				break;
			}
			if (st->id != MT_RM_CONSTRAINT_ID_ALL) {
				ret = MT_RM_STATUS_STOP;
			}
		} while (0);
	}
	return ret;
}
