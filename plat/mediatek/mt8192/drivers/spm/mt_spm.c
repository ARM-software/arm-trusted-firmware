/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <mt_lp_rm.h>
#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_conservation.h>
#include <mt_spm_constraint.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_suspend.h>
#include <mtk_plat_common.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <platform_def.h>
#include <sleep_def.h>

#ifdef MT_SPM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(spm_lock);
#define plat_spm_lock_init() bakery_lock_init(&spm_lock)
#else
spinlock_t spm_lock;
#define plat_spm_lock_init()
#endif

/* CLK_SCP_CFG_0 */
#define CLK_SCP_CFG_0		(TOPCKGEN_BASE + 0x200)
#define SPM_CK_CONTROL_EN	0x3FF

/* CLK_SCP_CFG_1 */
#define CLK_SCP_CFG_1		(TOPCKGEN_BASE + 0x210)
#define CLK_SCP_CFG_1_MASK	0x100C
#define CLK_SCP_CFG_1_SPM	0x3

struct mt_resource_constraint plat_constraint_bus26m = {
	.is_valid = spm_is_valid_rc_bus26m,
	.update = spm_update_rc_bus26m,
	.allow = spm_allow_rc_bus26m,
	.run = spm_run_rc_bus26m,
	.reset = spm_reset_rc_bus26m,
};

struct mt_resource_constraint plat_constraint_syspll = {
	.is_valid = spm_is_valid_rc_syspll,
	.update = spm_update_rc_syspll,
	.allow = spm_allow_rc_syspll,
	.run = spm_run_rc_syspll,
	.reset = spm_reset_rc_syspll,
};

struct mt_resource_constraint plat_constraint_dram = {
	.is_valid = spm_is_valid_rc_dram,
	.update = spm_update_rc_dram,
	.allow = spm_allow_rc_dram,
	.run = spm_run_rc_dram,
	.reset = spm_reset_rc_dram,
};

struct mt_resource_constraint plat_constraint_cpu = {
	.is_valid = spm_is_valid_rc_cpu_buck_ldo,
	.update = NULL,
	.allow = spm_allow_rc_cpu_buck_ldo,
	.run = spm_run_rc_cpu_buck_ldo,
	.reset = spm_reset_rc_cpu_buck_ldo,
};

struct mt_resource_constraint *plat_constraints[] = {
	&plat_constraint_bus26m,
	&plat_constraint_syspll,
	&plat_constraint_dram,
	&plat_constraint_cpu,
	NULL,
};

struct mt_resource_manager plat_mt8192_rm = {
	.update = mt_spm_cond_update,
	.consts = plat_constraints,
};

void spm_boot_init(void)
{
	/* switch ck_off/axi_26m control to SPM */
	mmio_setbits_32(CLK_SCP_CFG_0, SPM_CK_CONTROL_EN);
	mmio_clrsetbits_32(CLK_SCP_CFG_1, CLK_SCP_CFG_1_MASK,
			   CLK_SCP_CFG_1_SPM);

	plat_spm_lock_init();
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
	mt_lp_rm_register(&plat_mt8192_rm);
	mt_spm_idle_generic_init();
	mt_spm_suspend_init();
}
