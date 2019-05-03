/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* common headers */
#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <errno.h>

/* mediatek platform specific headers */
#include <platform_def.h>
#include <scu.h>
#include <mt_gic_v3.h>
#include <mtk_plat_common.h>
#include <mtgpio.h>
#include <mtspmc.h>
#include <plat_dcm.h>
#include <plat_debug.h>
#include <plat_params.h>
#include <plat_private.h>
#include <power_tracer.h>
#include <pmic.h>
#include <rtc.h>

#define MTK_LOCAL_STATE_OFF     2

static uintptr_t secure_entrypoint;

static void mp1_L2_desel_config(void)
{
	mmio_write_64(MCUCFG_BASE + 0x2200, 0x2092c820);

	dsb();
}

static int plat_mtk_power_domain_on(unsigned long mpidr)
{
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);

	INFO("%s():%d: mpidr: %lx, c.c: %d.%d\n",
		__func__, __LINE__, mpidr, cluster, cpu);

	/* power on cluster */
	if (!spm_get_cluster_powerstate(cluster)) {
		spm_poweron_cluster(cluster);
		if (cluster == 1) {
			l2c_parity_check_setup();
			circular_buffer_setup();
			mp1_L2_desel_config();
			mt_gic_sync_dcm_disable();
		}
	}

	/* init cpu reset arch as AARCH64 */
	mcucfg_init_archstate(cluster, cpu, 1);
	mcucfg_set_bootaddr(cluster, cpu, secure_entrypoint);

	spm_poweron_cpu(cluster, cpu);

	return PSCI_E_SUCCESS;
}

static void plat_mtk_power_domain_off(const psci_power_state_t *state)
{
	uint64_t mpidr = read_mpidr();
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);

	INFO("%s():%d: c.c: %d.%d\n", __func__, __LINE__, cluster, cpu);

	/* Prevent interrupts from spuriously waking up this cpu */
	mt_gic_cpuif_disable();

	spm_enable_cpu_auto_off(cluster, cpu);

	if (state->pwr_domain_state[MPIDR_AFFLVL1] == MTK_LOCAL_STATE_OFF) {
		if (cluster == 1)
			mt_gic_sync_dcm_enable();

		plat_mtk_cci_disable();
		spm_enable_cluster_auto_off(cluster);
	}

	spm_set_cpu_power_off(cluster, cpu);
}

static void plat_mtk_power_domain_on_finish(const psci_power_state_t *state)
{
	uint64_t mpidr = read_mpidr();
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);

	INFO("%s():%d: c.c: %d.%d\n", __func__, __LINE__, cluster, cpu);

	assert(state->pwr_domain_state[MPIDR_AFFLVL0] == MTK_LOCAL_STATE_OFF);

	if (state->pwr_domain_state[MPIDR_AFFLVL1] == MTK_LOCAL_STATE_OFF) {
		enable_scu(mpidr);

		/* Enable coherency if this cluster was off */
		plat_mtk_cci_enable();
		/* Enable big core dcm if this cluster was on */
		plat_dcm_restore_cluster_on(mpidr);
		/* Enable rgu dcm if this cluster was off */
		plat_dcm_rgu_enable();
	}

	spm_disable_cpu_auto_off(cluster, cpu);

	/* Enable the gic cpu interface */
	mt_gic_pcpu_init();
	mt_gic_cpuif_enable();
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_mtk_system_off(void)
{
	INFO("MTK System Off\n");

	rtc_power_off_sequence();
	wk_pmic_enable_sdn_delay();
	pmic_power_off();

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_mtk_system_reset(void)
{
	struct bl_aux_gpio_info *gpio_reset = plat_get_mtk_gpio_reset();

	INFO("MTK System Reset\n");

	mt_set_gpio_out(gpio_reset->index, gpio_reset->polarity);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static const plat_psci_ops_t plat_plat_pm_ops = {
	.cpu_standby			= NULL,
	.pwr_domain_on			= plat_mtk_power_domain_on,
	.pwr_domain_on_finish		= plat_mtk_power_domain_on_finish,
	.pwr_domain_off			= plat_mtk_power_domain_off,
	.pwr_domain_suspend		= NULL,
	.pwr_domain_suspend_finish	= NULL,
	.system_off			= plat_mtk_system_off,
	.system_reset			= plat_mtk_system_reset,
	.validate_power_state		= NULL,
	.get_sys_suspend_power_state	= NULL,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;
	secure_entrypoint = sec_entrypoint;
	return 0;
}
