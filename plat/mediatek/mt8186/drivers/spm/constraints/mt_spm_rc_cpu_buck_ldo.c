/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_notifier.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_suspend.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>

#define CONSTRAINT_CPU_BUCK_PCM_FLAG	\
	(SPM_FLAG_DISABLE_INFRA_PDN |	\
	 SPM_FLAG_DISABLE_VCORE_DVS |	\
	 SPM_FLAG_DISABLE_VCORE_DFS |	\
	 SPM_FLAG_SRAM_SLEEP_CTRL |	\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH)

#define CONSTRAINT_CPU_BUCK_PCM_FLAG1	(0U)

#define CONSTRAINT_CPU_BUCK_RESOURCE_REQ	\
	(MT_SPM_DRAM_S1 |			\
	 MT_SPM_DRAM_S0 |			\
	 MT_SPM_SYSPLL |			\
	 MT_SPM_INFRA |				\
	 MT_SPM_26M |				\
	 MT_SPM_XO_FPM)

static unsigned int cpubuckldo_status = MT_SPM_RC_VALID_SW;
static unsigned int cpubuckldo_enter_cnt;

static void spm_cpu_bcuk_ldo_conduct(struct spm_lp_scen *spm_lp,
				unsigned int *resource_req)
{
	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_CPU_BUCK_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_CPU_BUCK_PCM_FLAG1;
	*resource_req |= CONSTRAINT_CPU_BUCK_RESOURCE_REQ;
}

bool spm_is_valid_rc_cpu_buck_ldo(unsigned int cpu, int state_id)
{
	(void)cpu;
	(void)state_id;

	return IS_MT_RM_RC_READY(cpubuckldo_status);
}

unsigned int spm_allow_rc_cpu_buck_ldo(int state_id)
{
	(void)state_id;

	return MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF;
}

int spm_run_rc_cpu_buck_ldo(unsigned int cpu, int state_id)
{
	(void)cpu;
	unsigned int ext_op = 0U;

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER,
			       (IS_PLAT_SUSPEND_ID(state_id) ?
				(MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND) : (0U)));
#endif
	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     MT_SPM_EX_OP_SET_WDT,
				     CONSTRAINT_CPU_BUCK_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op,
					  spm_cpu_bcuk_ldo_conduct);
	}

	cpubuckldo_enter_cnt++;

	return 0;
}

int spm_reset_rc_cpu_buck_ldo(unsigned int cpu, int state_id)
{
	(void)cpu;
	unsigned int ext_op = 0U;

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, 0U);
#endif
	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_resume(state_id, MT_SPM_EX_OP_SET_WDT, NULL);
	} else {
		mt_spm_idle_generic_resume(state_id, ext_op, NULL, NULL);
	}

	return 0;
}
