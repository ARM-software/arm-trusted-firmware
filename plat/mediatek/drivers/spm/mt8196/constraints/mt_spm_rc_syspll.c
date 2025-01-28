/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <common/debug.h>

#include <constraints/mt_spm_rc_api.h>
#include <constraints/mt_spm_rc_internal.h>
#include <drivers/spm/mt_spm_resource_req.h>
#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_api.h>
#include <lpm_v2/mt_lp_rm.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_lp.h>
#include <mt_spm_reg.h>
#include <mt_spm_suspend.h>
#include <notifier/inc/mt_spm_notifier.h>

#define CONSTRAINT_SYSPLL_ALLOW (MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF | \
				 MT_RM_CONSTRAINT_ALLOW_DRAM_S0 | \
				 MT_RM_CONSTRAINT_ALLOW_DRAM_S1 | \
				 MT_RM_CONSTRAINT_ALLOW_VCORE_LP | \
				 MT_RM_CONSTRAINT_ALLOW_LVTS_STATE)

#define CONSTRAINT_SYSPLL_PCM_FLAG (SPM_FLAG_DISABLE_VCORE_DVS | \
				    SPM_FLAG_DISABLE_DDR_DFS | \
				    SPM_FLAG_DISABLE_EMI_DFS | \
				    SPM_FLAG_DISABLE_BUS_DFS | \
				    SPM_FLAG_ENABLE_AOV | \
				    SPM_FLAG_DISABLE_VLP_PDN | \
				    SPM_FLAG_SRAM_SLEEP_CTRL | \
				    SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_SYSPLL_PCM_FLAG1 (SPM_FLAG1_ENABLE_ALCO_TRACE)

#define CONSTRAINT_SYSPLL_RESOURCE_REQ	(MT_SPM_26M | \
					 MT_SPM_VCORE | MT_SPM_INFRA)

#define CHECK_VAL(val, sys_state) \
	((val) ? !!(*((uint32_t *)(val)) == (sys_state)) : 0)

static uint32_t cmd;

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_SYSPL,
	.is_valid = (MT_SPM_RC_VALID_SW |
		     MT_SPM_RC_VALID_FW |
		     MT_SPM_RC_VALID_NOTIFY),
	.enter_cnt = 0,
	.residency = 0,
};

int spm_syspll_conduct(int state_id, struct spm_lp_scen *spm_lp,
		       uint32_t *resource_req)
{
	if ((spm_lp == NULL) || (resource_req == NULL))
		return -1;

	struct pwr_ctrl *pwrctrl = spm_lp->pwrctrl;

	pwrctrl->pcm_flags = CONSTRAINT_SYSPLL_PCM_FLAG;
	pwrctrl->pcm_flags1 = CONSTRAINT_SYSPLL_PCM_FLAG1;

	*resource_req |= CONSTRAINT_SYSPLL_RESOURCE_REQ;

	return 0;
}

bool spm_is_valid_rc_syspll(uint32_t cpu, int state_id)
{
	return IS_MT_RM_RC_READY(status.is_valid);
}

int spm_update_rc_syspll(int state_id, int type, const void *val)
{
	int res = MT_RM_STATUS_OK;

	if (type == PLAT_RC_CLKBUF_STATUS) {
		bool is_flight = CHECK_VAL(val, FLIGHT_MODE_ON);

		if (is_flight)
			spm_rc_constraint_valid_set(MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_SPM_RC_VALID_FLIGHTMODE,
						    &status);
		else
			spm_rc_constraint_valid_clr(MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_SPM_RC_VALID_FLIGHTMODE,
						    &status);
	} else if (type == PLAT_RC_UFS_STATUS) {
		uint32_t is_ufs_h8 =  CHECK_VAL(val, UFS_REF_CLK_OFF);

		if (is_ufs_h8)
			spm_rc_constraint_valid_set(MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_SPM_RC_VALID_UFS_H8,
						    &status);
		else
			spm_rc_constraint_valid_clr(MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_RM_CONSTRAINT_ID_SYSPL,
						    MT_SPM_RC_VALID_UFS_H8,
						    &status);
	} else if (type == PLAT_RC_STATUS) {
		const struct rc_common_state *st;

		st = (const struct rc_common_state *)val;

		if (!st)
			return 0;
		if ((st->type == CONSTRAINT_UPDATE_VALID) ||
		    (st->type == CONSTRAINT_RESIDNECY))
			spm_rc_constraint_status_set(st->id, st->type,
						     st->act,
						     MT_RM_CONSTRAINT_ID_SYSPL,
						     st->value,
						     &status);
		else
			INFO("[%s:%d] - Unknown type: 0x%x\n",
			     __func__, __LINE__, st->type);
	}

	return res;
}

uint32_t spm_allow_rc_syspll(int state_id)
{
	return CONSTRAINT_SYSPLL_ALLOW;
}

int spm_run_rc_syspll(uint32_t cpu, int state_id)
{
	uint32_t ext_op = MT_SPM_EX_OP_HW_S1_DETECT |
			  MT_SPM_EX_OP_NOTIFY_INFRA_OFF;
	uint32_t nb_type = MT_SPM_NOTIFY_IDLE_ENTER;

	MT_SPM_RC_TAG(cpu, state_id, MT_RM_CONSTRAINT_ID_SYSPL);
	MT_SPM_RC_TAG_VALID(status.is_valid);
	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_START);

	cmd = CONSTRAINT_SYSPLL_ALLOW;

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		cmd |= MT_RM_CONSTRAINT_ALLOW_AP_PLAT_SUSPEND;
		ext_op |= (MT_SPM_EX_OP_CLR_26M_RECORD |
			   MT_SPM_EX_OP_SET_WDT);
	} else {
		if (status.is_valid & MT_SPM_RC_VALID_TRACE_TIME)
			ext_op |= MT_SPM_EX_OP_TRACE_TIMESTAMP_EN;

		if (!IS_MT_PLAT_PWR_STATE(state_id,
					  MT_PLAT_PWR_STATE_SYSTEM_PLL)) {
			nb_type = MT_SPM_NOTIFY_IDLE_ENTER;
			ext_op &= ~MT_SPM_EX_OP_NOTIFY_INFRA_OFF;
		}
	}

#ifdef MTK_SPM_PMIC_LP_SUPPORT
	do_spm_low_power(SPM_LP_ENTER, cmd);
#endif

	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_NOTIFY);

	if (IS_MT_SPM_RC_NOTIFY_ENABLE(status.is_valid))
		mt_spm_sspm_notify_u32(nb_type, cmd);

	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_WAKE_SPM_BEFORE);

	if (IS_PLAT_SUSPEND_ID(state_id))
		mt_spm_suspend_enter(state_id, ext_op,
				     CONSTRAINT_SYSPLL_RESOURCE_REQ);
	else
		mt_spm_idle_generic_enter(state_id, ext_op, spm_syspll_conduct);

	MT_SPM_RC_FP(MT_SPM_RC_FP_ENTER_WAKE_SPM_AFTER);
	return 0;
}

int spm_reset_rc_syspll(uint32_t cpu, int state_id)
{
	uint32_t ext_op = MT_SPM_EX_OP_HW_S1_DETECT |
			  MT_SPM_EX_OP_NOTIFY_INFRA_OFF;
	uint32_t nb_type = MT_SPM_NOTIFY_IDLE_LEAVE;

	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_START);

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		ext_op |= (MT_SPM_EX_OP_SET_WDT);
	} else {
		if (status.is_valid & MT_SPM_RC_VALID_TRACE_TIME)
			ext_op |= MT_SPM_EX_OP_TRACE_TIMESTAMP_EN;

		if (spm_unlikely(status.is_valid &
		    MT_SPM_RC_VALID_TRACE_EVENT))
			ext_op |= MT_SPM_EX_OP_TRACE_LP;

		if (!IS_MT_PLAT_PWR_STATE(state_id,
		    MT_PLAT_PWR_STATE_SYSTEM_PLL)) {
			nb_type = MT_SPM_NOTIFY_IDLE_LEAVE;
			ext_op &= ~MT_SPM_EX_OP_NOTIFY_INFRA_OFF;
		}
	}

#ifdef MTK_SPM_PMIC_LP_SUPPORT
	do_spm_low_power(SPM_LP_RESUME, cmd);
#endif

	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_NOTIFY);

	if (IS_MT_SPM_RC_NOTIFY_ENABLE(status.is_valid))
		mt_spm_sspm_notify_u32(nb_type, cmd);

	MT_SPM_RC_FP(MT_SPM_RC_FP_RESUME_RESET_SPM_BEFORE);

	if (IS_PLAT_SUSPEND_ID(state_id))
		mt_spm_suspend_resume(state_id, ext_op, NULL);
	else {
		struct wake_status *waken = NULL;

		mt_spm_idle_generic_resume(state_id, ext_op, &waken, NULL);
		status.enter_cnt++;

		if (spm_unlikely(status.is_valid & MT_SPM_RC_VALID_RESIDNECY))
			status.residency += waken ?
					    waken->tr.comm.timer_out : 0;
	}

	MT_SPM_RC_FP(MT_SPM_RC_FP_INIT);
	return 0;
}

int spm_get_status_rc_syspll(uint32_t type, void *priv)
{
	int ret = MT_RM_STATUS_OK;

	if (type == PLAT_RC_STATUS) {
		int res = 0;
		struct rc_common_state *st = (struct rc_common_state *)priv;

		if (!st)
			return MT_RM_STATUS_BAD;

		res = spm_rc_constraint_status_get(st->id, st->type,
						   st->act,
						   MT_RM_CONSTRAINT_ID_SYSPL,
						   &status,
						   st->value);
		if (!res && (st->id != MT_RM_CONSTRAINT_ID_ALL))
			ret = MT_RM_STATUS_STOP;
	}
	return ret;
}
