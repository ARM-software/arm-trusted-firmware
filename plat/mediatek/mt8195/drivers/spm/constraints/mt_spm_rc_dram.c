/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#include <mt_lp_rm.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_constraint.h>
#include <mt_spm_conservation.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_notifier.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_reg.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_suspend.h>
#include <plat_pm.h>
#include <plat_mtk_lpm.h>

#define CONSTRAINT_DRAM_ALLOW			\
	(MT_RM_CONSTRAINT_ALLOW_DRAM_S0	|	\
	 MT_RM_CONSTRAINT_ALLOW_DRAM_S1 |	\
	 MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF)

#define CONSTRAINT_DRAM_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_SRAM_SLEEP_CTRL |		\
	 SPM_FLAG_KEEP_CSYSPWRACK_HIGH |	\
	 SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP)

#define CONSTRAINT_DRAM_PCM_FLAG1		0U

#define CONSTRAINT_DRAM_RESOURCE_REQ		\
	(MT_SPM_SYSPLL |			\
	 MT_SPM_INFRA |				\
	 MT_SPM_26M)

static struct mt_spm_cond_tables cond_dram = {
	.name = "dram",
	.table_cg = {
		0xFFFDD008,	/* MTCMOS1 */
		0x20040802,	/* INFRA0  */
		0x27AF8000,	/* INFRA1  */
		0x86040640,	/* INFRA2  */
		0x00000000,	/* INFRA3  */
		0x80000000,	/* INFRA4  */
		0x00000000,	/* PERI0   */
		0x00004000,	/* VPPSYS0_0  */
		0x08803000,	/* VPPSYS0_1  */
		0x00000000,	/* VPPSYS0_2  */
		0x80005555,	/* VPPSYS1_0  */
		0x00009008,	/* VPPSYS1_1  */
		0x60060000,	/* VDOSYS0_0  */
		0x00000000,	/* VDOSYS0_1  */
		0x201E01F8,	/* VDOSYS1_0  */
		0x00800000,	/* VDOSYS1_1  */
		0x00000000,	/* VDOSYS1_2  */
		0x00000080,	/* I2C */
	},
	.table_pll = 0U,
};

static struct mt_spm_cond_tables cond_dram_res = {
	.table_cg = { 0U },
	.table_pll = 0U,
};

static struct constraint_status status = {
	.id = MT_RM_CONSTRAINT_ID_DRAM,
	.valid = (MT_SPM_RC_VALID_SW |
		  MT_SPM_RC_VALID_COND_LATCH |
		  MT_SPM_RC_VALID_XSOC_BBLPM),
	.cond_block = 0U,
	.enter_cnt = 0U,
	.cond_res = &cond_dram_res,
};

static void spm_dram_conduct(struct spm_lp_scen *spm_lp,
			     unsigned int *resource_req)
{
	spm_lp->pwrctrl->pcm_flags = (uint32_t)CONSTRAINT_DRAM_PCM_FLAG;
	spm_lp->pwrctrl->pcm_flags1 = (uint32_t)CONSTRAINT_DRAM_PCM_FLAG1;
	*resource_req |= CONSTRAINT_DRAM_RESOURCE_REQ;
}

bool spm_is_valid_rc_dram(unsigned int cpu, int state_id)
{
	(void)cpu;
	(void)state_id;

	return (status.cond_block == 0U) && IS_MT_RM_RC_READY(status.valid);
}

int spm_update_rc_dram(int state_id, int type, const void *val)
{
	const struct mt_spm_cond_tables *tlb;
	const struct mt_spm_cond_tables *tlb_check;
	int res = MT_RM_STATUS_OK;

	if (val == NULL) {
		return MT_RM_STATUS_BAD;
	}

	if (type == PLAT_RC_UPDATE_CONDITION) {
		tlb = (const struct mt_spm_cond_tables *)val;
		tlb_check = (const struct mt_spm_cond_tables *)&cond_dram;
		status.cond_block =
			mt_spm_cond_check(state_id, tlb, tlb_check,
					  ((status.valid &
					    MT_SPM_RC_VALID_COND_LATCH) != 0U) ?
					  &cond_dram_res : NULL);
	} else {
		res = MT_RM_STATUS_BAD;
	}

	return res;
}

unsigned int spm_allow_rc_dram(int state_id)
{
	(void)state_id;

	return CONSTRAINT_DRAM_ALLOW;
}

int spm_run_rc_dram(unsigned int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;
	unsigned int allows = CONSTRAINT_DRAM_ALLOW;

	(void)cpu;

	if (IS_MT_SPM_RC_BBLPM_MODE(status.valid)) {
#ifdef MT_SPM_USING_SRCLKEN_RC
		ext_op |= MT_SPM_EX_OP_SRCLKEN_RC_BBLPM;
#else
		allows |= MT_RM_CONSTRAINT_ALLOW_BBLPM;
#endif
	}

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_ENTER, allows |
			       (IS_PLAT_SUSPEND_ID(state_id) ?
				MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND : 0U));
#else
	(void)allows;
#endif

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_enter(state_id,
				     (MT_SPM_EX_OP_SET_WDT |
				      MT_SPM_EX_OP_SET_SUSPEND_MODE |
				      MT_SPM_EX_OP_HW_S1_DETECT),
				     CONSTRAINT_DRAM_RESOURCE_REQ);
	} else {
		mt_spm_idle_generic_enter(state_id, ext_op, spm_dram_conduct);
	}

	return 0;
}

int spm_reset_rc_dram(unsigned int cpu, int state_id)
{
	unsigned int ext_op = MT_SPM_EX_OP_HW_S1_DETECT;
	unsigned int allows = CONSTRAINT_DRAM_ALLOW;

	(void)cpu;

	if (IS_MT_SPM_RC_BBLPM_MODE(status.valid)) {
#ifdef MT_SPM_USING_SRCLKEN_RC
		ext_op |= MT_SPM_EX_OP_SRCLKEN_RC_BBLPM;
#else
		allows |= MT_RM_CONSTRAINT_ALLOW_BBLPM;
#endif
	}

#ifndef ATF_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
	mt_spm_sspm_notify_u32(MT_SPM_NOTIFY_LP_LEAVE, allows);
#else
	(void)allows;
#endif

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		mt_spm_suspend_resume(state_id,
				      (MT_SPM_EX_OP_SET_WDT |
				       MT_SPM_EX_OP_HW_S1_DETECT),
				      NULL);
	} else {
		mt_spm_idle_generic_resume(state_id, ext_op, NULL);
		status.enter_cnt++;
	}

	return 0;
}
