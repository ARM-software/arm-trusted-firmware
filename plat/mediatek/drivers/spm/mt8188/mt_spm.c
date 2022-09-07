/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <arch.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include "constraints/mt_spm_rc_internal.h"
#include <drivers/spm/mt_spm_resource_req.h>
#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lp_rm.h>
#include <lpm/mt_lp_rqm.h>
#include <lpm/mt_lpm_smc.h>
#include "mt_spm.h"
#include "mt_spm_cond.h"
#include "mt_spm_conservation.h"
#include "mt_spm_constraint.h"
#include "mt_spm_idle.h"
#include "mt_spm_internal.h"
#include "mt_spm_pmic_wrap.h"
#include "mt_spm_reg.h"
#include "mt_spm_suspend.h"
#include <mtk_mmap_pool.h>
#include <platform_def.h>
#include "sleep_def.h"

/*
 * System Power Manager (SPM) is a hardware module which provides CPU idle
 * and system suspend features.
 */

spinlock_t spm_lock;

#ifdef MTK_PLAT_SPM_UNSUPPORT
struct mt_resource_manager plat_mt8188_rm = {
};
#else
struct mt_lp_res_req rq_xo_fpm = {
	.res_id = MT_LP_RQ_XO_FPM,
	.res_rq = MT_SPM_XO_FPM,
	.res_usage = 0,
};

struct mt_lp_res_req rq_26m = {
	.res_id = MT_LP_RQ_26M,
	.res_rq = MT_SPM_26M,
	.res_usage = 0,
};

struct mt_lp_res_req rq_infra = {
	.res_id = MT_LP_RQ_INFRA,
	.res_rq = MT_SPM_INFRA,
	.res_usage = 0,
};

struct mt_lp_res_req rq_syspll = {
	.res_id = MT_LP_RQ_SYSPLL,
	.res_rq = MT_SPM_SYSPLL,
	.res_usage = 0,
};

struct mt_lp_res_req rq_dram_s0 = {
	.res_id = MT_LP_RQ_DRAM,
	.res_rq = MT_SPM_DRAM_S0,
	.res_usage = 0,
};

struct mt_lp_res_req rq_dram_s1 = {
	.res_id = MT_LP_RQ_DRAM,
	.res_rq = MT_SPM_DRAM_S1,
	.res_usage = 0,
};

struct mt_lp_res_req *spm_resources[] = {
	&rq_xo_fpm,
	&rq_26m,
	&rq_infra,
	&rq_syspll,
	&rq_dram_s0,
	&rq_dram_s1,
	NULL,
};

struct mt_resource_req_manager plat_mt8188_rq = {
	.res = spm_resources,
};

struct mt_resource_constraint plat_constraint_bus26m = {
	.is_valid = spm_is_valid_rc_bus26m,
	.update = spm_update_rc_bus26m,
	.allow = spm_allow_rc_bus26m,
	.run = spm_run_rc_bus26m,
	.reset = spm_reset_rc_bus26m,
	.get_status = spm_get_status_rc_bus26m,
};

struct mt_resource_constraint plat_constraint_syspll = {
	.is_valid = spm_is_valid_rc_syspll,
	.update = spm_update_rc_syspll,
	.allow = spm_allow_rc_syspll,
	.run = spm_run_rc_syspll,
	.reset = spm_reset_rc_syspll,
	.get_status = spm_get_status_rc_syspll,
};

struct mt_resource_constraint plat_constraint_dram = {
	.is_valid = spm_is_valid_rc_dram,
	.update = spm_update_rc_dram,
	.allow = spm_allow_rc_dram,
	.run = spm_run_rc_dram,
	.reset = spm_reset_rc_dram,
	.get_status = spm_get_status_rc_dram,
};

struct mt_resource_constraint plat_constraint_cpu = {
	.is_valid = spm_is_valid_rc_cpu_buck_ldo,
	.update = spm_update_rc_cpu_buck_ldo,
	.allow = spm_allow_rc_cpu_buck_ldo,
	.run = spm_run_rc_cpu_buck_ldo,
	.reset = spm_reset_rc_cpu_buck_ldo,
	.get_status = spm_get_status_rc_cpu_buck_ldo,
};

struct mt_resource_constraint *plat_constraints[] = {
	&plat_constraint_bus26m,
	&plat_constraint_syspll,
	&plat_constraint_dram,
	&plat_constraint_cpu,
	NULL,
};

struct mt_resource_manager plat_mt8188_rm = {
	.update = mt_spm_cond_update,
	.consts = plat_constraints,
};
#endif

/* Determine for SPM software resource user */
static struct mt_lp_resource_user spm_res_user;

struct mt_lp_resource_user *get_spm_res_user(void)
{
	return &spm_res_user;
}

int spm_boot_init(void)
{
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
	mt_lp_rm_register(&plat_mt8188_rm);

	/* SPM service won't run when SPM not ready */
#ifndef MTK_PLAT_SPM_UNSUPPORT
	mt_lp_resource_request_manager_register(&plat_mt8188_rq);
	mt_lp_resource_user_register("SPM", &spm_res_user);
#endif

	return 0;
}
MTK_ARCH_INIT(spm_boot_init);
