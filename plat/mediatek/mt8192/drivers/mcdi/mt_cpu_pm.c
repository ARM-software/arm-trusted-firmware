/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>

#include <mt_cpu_pm_cpc.h>
#include <mt_lp_irqremain.h>
#include <mt_lp_rm.h>
#include <mt_mcdi.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>

DEFINE_SYSREG_RW_FUNCS(dbgprcr_el1);

static int plat_mt_lp_cpu_rc;

static int pwr_state_prompt(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_state_reflect(unsigned int cpu, const psci_power_state_t *state)
{
	mtk_cpc_core_on_hint_clr(cpu);

	if (IS_SYSTEM_SUSPEND_STATE(state)) {
		mtk_cpc_time_sync();
	}

	return 0;
}

static int pwr_cpu_pwron(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_cpu_pwrdwn(unsigned int cpu, const psci_power_state_t *state)
{
	/* clear DBGPRCR.CORENPDRQ to allow CPU power down  */
	write_dbgprcr_el1(0ULL);

	return 0;
}

static int pwr_cluster_pwron(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_cluster_pwrdwn(unsigned int cpu, const psci_power_state_t *state)
{
	return 0;
}

static int pwr_mcusys_pwron(unsigned int cpu, const psci_power_state_t *state)
{
	if (!IS_MCUSYS_OFF_STATE(state) || (plat_mt_lp_cpu_rc < 0)) {
		return -1;
	}

	mtk_cpc_mcusys_off_reflect();

	return 0;
}

static int pwr_mcusys_pwron_finished(unsigned int cpu,
					const psci_power_state_t *state)
{
	int state_id = state->pwr_domain_state[MTK_AFFLVL_MCUSYS];

	if (!IS_MCUSYS_OFF_STATE(state) || (plat_mt_lp_cpu_rc < 0)) {
		return -1;
	}

	mt_lp_rm_reset_constraint(plat_mt_lp_cpu_rc, cpu, state_id);
	mt_lp_irqremain_release();

	return 0;
}

static int pwr_mcusys_pwrdwn(unsigned int cpu, const psci_power_state_t *state)
{
	int state_id = state->pwr_domain_state[MTK_AFFLVL_MCUSYS];

	if (!IS_MCUSYS_OFF_STATE(state)) {
		goto mt_pwr_mcusysoff_break;
	}

	if (mcdi_try_init() != 0) {
		goto mt_pwr_mcusysoff_break;
	}

	if (mtk_cpc_mcusys_off_prepare() != CPC_SUCCESS) {
		goto mt_pwr_mcusysoff_break;
	}

	plat_mt_lp_cpu_rc =
		mt_lp_rm_find_and_run_constraint(0, cpu, state_id, NULL);

	if (plat_mt_lp_cpu_rc < 0) {
		goto mt_pwr_mcusysoff_reflect;
	}

	mt_lp_irqremain_aquire();

	return 0;

mt_pwr_mcusysoff_reflect:
	mtk_cpc_mcusys_off_reflect();

mt_pwr_mcusysoff_break:
	plat_mt_lp_cpu_rc = -1;

	return -1;
}

static const struct mt_lpm_tz plat_pm = {
	.pwr_prompt			= pwr_state_prompt,
	.pwr_reflect			= pwr_state_reflect,
	.pwr_cpu_on			= pwr_cpu_pwron,
	.pwr_cpu_dwn			= pwr_cpu_pwrdwn,
	.pwr_cluster_on			= pwr_cluster_pwron,
	.pwr_cluster_dwn		= pwr_cluster_pwrdwn,
	.pwr_mcusys_dwn			= pwr_mcusys_pwrdwn,
	.pwr_mcusys_on			= pwr_mcusys_pwron,
	.pwr_mcusys_on_finished		= pwr_mcusys_pwron_finished
};

const struct mt_lpm_tz *mt_plat_cpu_pm_init(void)
{
	mtk_cpc_init();

	if (mcdi_try_init() == 0) {
		INFO("MCDI init done.\n");
	}

	mt_lp_irqremain_init();

	return &plat_pm;
}
