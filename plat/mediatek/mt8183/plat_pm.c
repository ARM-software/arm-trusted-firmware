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
#include <mtk_mcdi.h>
#include <mtk_plat_common.h>
#include <mtgpio.h>
#include <mtspmc.h>
#include <plat_dcm.h>
#include <plat_debug.h>
#include <plat_params.h>
#include <plat_private.h>
#include <power_tracer.h>
#include <pmic.h>
#include <spm.h>
#include <spm_suspend.h>
#include <sspm.h>
#include <rtc.h>

/* Local power state for power domains in Run state. */
#define MTK_LOCAL_STATE_RUN	0
/* Local power state for retention. */
#define MTK_LOCAL_STATE_RET	1
/* Local power state for OFF/power-down. */
#define MTK_LOCAL_STATE_OFF	2

#if PSCI_EXTENDED_STATE_ID
/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define MTK_LOCAL_PSTATE_WIDTH		4
#define MTK_LOCAL_PSTATE_MASK		((1 << MTK_LOCAL_PSTATE_WIDTH) - 1)

/* Macros to construct the composite power state */

/* Make composite power state parameter till power level 0 */

#define mtk_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
	(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))

#else /* !PSCI_EXTENDED_STATE_ID */

#define mtk_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		((type) << PSTATE_TYPE_SHIFT))

#endif /* PSCI_EXTENDED_STATE_ID */

/* Make composite power state parameter till power level 1 */
#define mtk_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << MTK_LOCAL_PSTATE_WIDTH) | \
		mtk_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/* Make composite power state parameter till power level 2 */
#define mtk_make_pwrstate_lvl2( \
		lvl2_state, lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl2_state) << (MTK_LOCAL_PSTATE_WIDTH * 2)) | \
		mtk_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type))

#define MTK_PWR_LVL0	0
#define MTK_PWR_LVL1	1
#define MTK_PWR_LVL2	2

/* Macros to read the MTK power domain state */
#define MTK_CORE_PWR_STATE(state)	(state)->pwr_domain_state[MTK_PWR_LVL0]
#define MTK_CLUSTER_PWR_STATE(state)	(state)->pwr_domain_state[MTK_PWR_LVL1]
#define MTK_SYSTEM_PWR_STATE(state)	((PLAT_MAX_PWR_LVL > MTK_PWR_LVL1) ? \
			(state)->pwr_domain_state[MTK_PWR_LVL2] : 0)

#if PSCI_EXTENDED_STATE_ID
/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
const unsigned int mtk_pm_idle_states[] = {
	/* State-id - 0x001 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_RUN, MTK_LOCAL_STATE_RUN,
		MTK_LOCAL_STATE_RET, MTK_PWR_LVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x002 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_RUN, MTK_LOCAL_STATE_RUN,
		MTK_LOCAL_STATE_OFF, MTK_PWR_LVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x022 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_RUN, MTK_LOCAL_STATE_OFF,
		MTK_LOCAL_STATE_OFF, MTK_PWR_LVL1, PSTATE_TYPE_POWERDOWN),
#if PLAT_MAX_PWR_LVL > MTK_PWR_LVL1
	/* State-id - 0x222 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_OFF, MTK_LOCAL_STATE_OFF,
		MTK_LOCAL_STATE_OFF, MTK_PWR_LVL2, PSTATE_TYPE_POWERDOWN),
#endif
	0,
};
#endif

#define CPU_IDX(cluster, cpu)		((cluster << 2) + cpu)
#define ON	true
#define OFF	false

/* Pause MCDI when CPU hotplug */
static bool HP_SSPM_PAUSE;
/* CPU Hotplug by SSPM */
static bool HP_SSPM_CTRL = true;
/* Turn off cluster when CPU hotplug off */
static bool HP_CLUSTER_OFF = true;
/* Turn off cluster when CPU MCDI off */
static bool MCDI_C2 = true;
/* Enable MCDI */
static bool MCDI_SSPM = true;

static uintptr_t secure_entrypoint;

static void mp1_L2_desel_config(void)
{
	mmio_write_64(MCUCFG_BASE + 0x2200, 0x2092c820);

	dsb();
}

static bool clst_single_pwr(int cluster, int cpu)
{
	uint32_t cpu_mask[2] = {0x00001e00, 0x000f0000};
	uint32_t cpu_pwr_bit[] = {9, 10, 11, 12, 16, 17, 18, 19};
	int my_idx = (cluster << 2) + cpu;
	uint32_t pwr_stat = mmio_read_32(0x10006180);

	return !(pwr_stat & (cpu_mask[cluster] & ~BIT(cpu_pwr_bit[my_idx])));
}

static bool clst_single_on(int cluster, int cpu)
{
	uint32_t cpu_mask[2] = {0x0f, 0xf0};
	int my_idx = (cluster << 2) + cpu;
	uint32_t on_stat = mcdi_avail_cpu_mask_read();

	return !(on_stat & (cpu_mask[cluster] & ~BIT(my_idx)));
}

static void plat_cpu_pwrdwn_common(void)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	mt_gic_rdistif_save();
	mt_gic_cpuif_disable();
}

static void plat_cpu_pwron_common(void)
{
	/* Enable the gic cpu interface */
	mt_gic_cpuif_enable();
	mt_gic_rdistif_init();
	mt_gic_rdistif_restore();
}

static void plat_cluster_pwrdwn_common(uint64_t mpidr, int cluster)
{
	if (cluster > 0)
		mt_gic_sync_dcm_enable();

	/* Disable coherency */
	plat_mtk_cci_disable();
	disable_scu(mpidr);
}

static void plat_cluster_pwron_common(uint64_t mpidr, int cluster)
{
	if (cluster > 0) {
		l2c_parity_check_setup();
		circular_buffer_setup();
		mp1_L2_desel_config();
		mt_gic_sync_dcm_disable();
	}

	/* Enable coherency */
	enable_scu(mpidr);
	plat_mtk_cci_enable();
	/* Enable big core dcm */
	plat_dcm_restore_cluster_on(mpidr);
	/* Enable rgu dcm */
	plat_dcm_rgu_enable();
}

static void plat_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);

	isb();
	dsb();
	wfi();

	write_scr_el3(scr);
}

static void mcdi_ctrl_before_hotplug_on(int cluster, int cpu)
{
	if (!HP_SSPM_CTRL && HP_SSPM_PAUSE && MCDI_SSPM) {
		mcdi_pause_clr(cluster, CPU_IDX(cluster, cpu), OFF);
		mcdi_pause_set(cluster, CPU_IDX(cluster, cpu), ON);
	}
}

static void mcdi_ctrl_before_hotplug_off(int cluster, int cpu, bool cluster_off)
{
	if (!HP_SSPM_CTRL && HP_SSPM_PAUSE && MCDI_SSPM)
		mcdi_pause_set(cluster_off ? cluster : -1,
				CPU_IDX(cluster, cpu), OFF);
}

static void mcdi_ctrl_cluster_cpu_off(int cluster, int cpu, bool cluster_off)
{
	if (MCDI_SSPM) {
		sspm_set_bootaddr(secure_entrypoint);

		sspm_standbywfi_irq_enable(CPU_IDX(cluster, cpu));

		if (cluster_off)
			sspm_cluster_pwr_off_notify(cluster);
		else
			sspm_cluster_pwr_on_notify(cluster);
	}
}

static void mcdi_ctrl_suspend(void)
{
	if (MCDI_SSPM)
		mcdi_pause();
}

static void mcdi_ctrl_resume(void)
{
	if (MCDI_SSPM)
		mcdi_unpause();
}

static void hotplug_ctrl_cluster_on(int cluster, int cpu)
{
	if (HP_SSPM_CTRL && MCDI_SSPM) {
		mcdi_hotplug_clr(cluster, CPU_IDX(cluster, cpu), OFF);
		mcdi_hotplug_set(cluster, -1, ON);
		mcdi_hotplug_wait_ack(cluster, -1, ON);
	} else {
		/* power on cluster */
		if (!spm_get_cluster_powerstate(cluster))
			spm_poweron_cluster(cluster);
	}
}

static void hotplug_ctrl_cpu_on(int cluster, int cpu)
{
	if (HP_SSPM_CTRL && MCDI_SSPM)
		mcdi_hotplug_set(cluster, CPU_IDX(cluster, cpu), ON);
	else
		spm_poweron_cpu(cluster, cpu);
}

static void hotplug_ctrl_cpu_on_finish(int cluster, int cpu)
{
	spm_disable_cpu_auto_off(cluster, cpu);

	if (HP_SSPM_CTRL && MCDI_SSPM)
		mcdi_hotplug_clr(cluster, CPU_IDX(cluster, cpu), ON);
	else if (HP_SSPM_PAUSE && MCDI_SSPM)
		mcdi_pause_clr(cluster, CPU_IDX(cluster, cpu), ON);

	mcdi_avail_cpu_mask_set(BIT(CPU_IDX(cluster, cpu)));
}

static void hotplug_ctrl_cluster_cpu_off(int cluster, int cpu, bool cluster_off)
{
	mcdi_avail_cpu_mask_clr(BIT(CPU_IDX(cluster, cpu)));

	if (HP_SSPM_CTRL && MCDI_SSPM) {
		mcdi_hotplug_set(cluster_off ? cluster : -1,
				CPU_IDX(cluster, cpu), OFF);
	} else {
		spm_enable_cpu_auto_off(cluster, cpu);

		if (cluster_off)
			spm_enable_cluster_auto_off(cluster);

		spm_set_cpu_power_off(cluster, cpu);
	}
}

static int plat_mtk_power_domain_on(unsigned long mpidr)
{
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	int clst_pwr = spm_get_cluster_powerstate(cluster);
	unsigned int i;

	mcdi_ctrl_before_hotplug_on(cluster, cpu);
	hotplug_ctrl_cluster_on(cluster, cpu);

	if (clst_pwr == 0) {
		/* init cpu reset arch as AARCH64 of cluster */
		for (i = 0; i < PLATFORM_MAX_CPUS_PER_CLUSTER; i++) {
			mcucfg_init_archstate(cluster, i, 1);
			mcucfg_set_bootaddr(cluster, i, secure_entrypoint);
		}
	}

	hotplug_ctrl_cpu_on(cluster, cpu);

	return PSCI_E_SUCCESS;
}

static void plat_mtk_power_domain_off(const psci_power_state_t *state)
{
	uint64_t mpidr = read_mpidr();
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	const plat_local_state_t *pds = state->pwr_domain_state;
	bool afflvl1 = (pds[MPIDR_AFFLVL1] == MTK_LOCAL_STATE_OFF);
	bool cluster_off = (HP_CLUSTER_OFF && afflvl1 &&
					clst_single_on(cluster, cpu));

	plat_cpu_pwrdwn_common();

	if (cluster_off)
		plat_cluster_pwrdwn_common(mpidr, cluster);

	mcdi_ctrl_before_hotplug_off(cluster, cpu, cluster_off);
	hotplug_ctrl_cluster_cpu_off(cluster, cpu, cluster_off);
}

static void plat_mtk_power_domain_on_finish(const psci_power_state_t *state)
{
	uint64_t mpidr = read_mpidr();
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	const plat_local_state_t *pds = state->pwr_domain_state;
	bool afflvl1 = (pds[MPIDR_AFFLVL1] == MTK_LOCAL_STATE_OFF);

	if (afflvl1)
		plat_cluster_pwron_common(mpidr, cluster);

	plat_cpu_pwron_common();

	hotplug_ctrl_cpu_on_finish(cluster, cpu);
}

static void plat_mtk_power_domain_suspend(const psci_power_state_t *state)
{
	uint64_t mpidr = read_mpidr();
	int cpu = MPIDR_AFFLVL0_VAL(mpidr);
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	const plat_local_state_t *pds = state->pwr_domain_state;
	bool afflvl1 = (pds[MPIDR_AFFLVL1] == MTK_LOCAL_STATE_OFF);
	bool afflvl2 = (pds[MPIDR_AFFLVL2] == MTK_LOCAL_STATE_OFF);
	bool cluster_off = MCDI_C2 && afflvl1 && clst_single_pwr(cluster, cpu);

	plat_cpu_pwrdwn_common();

	plat_dcm_mcsi_a_backup();

	if (cluster_off || afflvl2)
		plat_cluster_pwrdwn_common(mpidr, cluster);

	if (afflvl2) {
		spm_data_t spm_d = { .cmd = SPM_SUSPEND };
		uint32_t *d = (uint32_t *)&spm_d;
		uint32_t l = sizeof(spm_d) / sizeof(uint32_t);

		mcdi_ctrl_suspend();

		spm_set_bootaddr(secure_entrypoint);

		if (MCDI_SSPM)
			sspm_ipi_send_non_blocking(IPI_ID_SUSPEND, d);

		spm_system_suspend();

		if (MCDI_SSPM)
			while (sspm_ipi_recv_non_blocking(IPI_ID_SUSPEND, d, l))
				;

		mt_gic_distif_save();
	} else {
		mcdi_ctrl_cluster_cpu_off(cluster, cpu, cluster_off);
	}
}

static void plat_mtk_power_domain_suspend_finish(const psci_power_state_t *state)
{
	uint64_t mpidr = read_mpidr();
	int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	const plat_local_state_t *pds = state->pwr_domain_state;
	bool afflvl2 = (pds[MPIDR_AFFLVL2] == MTK_LOCAL_STATE_OFF);

	if (afflvl2) {
		spm_data_t spm_d = { .cmd = SPM_RESUME };
		uint32_t *d = (uint32_t *)&spm_d;
		uint32_t l = sizeof(spm_d) / sizeof(uint32_t);

		mt_gic_init();
		mt_gic_distif_restore();
		mt_gic_rdistif_restore();

		mmio_write_32(EMI_WFIFO, 0xf);

		if (MCDI_SSPM)
			sspm_ipi_send_non_blocking(IPI_ID_SUSPEND, d);

		spm_system_suspend_finish();

		if (MCDI_SSPM)
			while (sspm_ipi_recv_non_blocking(IPI_ID_SUSPEND, d, l))
				;

		mcdi_ctrl_resume();
	} else {
		plat_cpu_pwron_common();
	}

	plat_cluster_pwron_common(mpidr, cluster);

	plat_dcm_mcsi_a_restore();
}

#if PSCI_EXTENDED_STATE_ID

static int plat_mtk_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	assert(req_state);

	if (!MCDI_SSPM)
		return PSCI_E_INVALID_PARAMS;

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!mtk_pm_idle_states[i]; i++) {
		if (power_state == mtk_pm_idle_states[i])
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!mtk_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
						MTK_LOCAL_PSTATE_MASK;
		state_id >>= MTK_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}

#else /* if !PSCI_EXTENDED_STATE_ID */

static int plat_mtk_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	assert(req_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != 0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MTK_PWR_LVL0] = MTK_LOCAL_STATE_RET;
	} else if (!MCDI_SSPM) {
		return PSCI_E_INVALID_PARAMS;
	} else {
		for (i = 0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] = MTK_LOCAL_STATE_OFF;
	}

	return PSCI_E_SUCCESS;
}

#endif /* PSCI_EXTENDED_STATE_ID */

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

static void plat_mtk_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	assert(PLAT_MAX_PWR_LVL >= 2);

	for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = MTK_LOCAL_STATE_OFF;
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static const plat_psci_ops_t plat_plat_pm_ops = {
	.cpu_standby			= plat_cpu_standby,
	.pwr_domain_on			= plat_mtk_power_domain_on,
	.pwr_domain_on_finish		= plat_mtk_power_domain_on_finish,
	.pwr_domain_off			= plat_mtk_power_domain_off,
	.pwr_domain_suspend		= plat_mtk_power_domain_suspend,
	.pwr_domain_suspend_finish	= plat_mtk_power_domain_suspend_finish,
	.system_off			= plat_mtk_system_off,
	.system_reset			= plat_mtk_system_reset,
	.validate_power_state		= plat_mtk_validate_power_state,
	.get_sys_suspend_power_state	= plat_mtk_get_sys_suspend_power_state
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	unsigned int i;

	*psci_ops = &plat_plat_pm_ops;
	secure_entrypoint = sec_entrypoint;

	/* Init cpu reset arch as AARCH64 of cluster 0 */
	for (i = 0; i < PLATFORM_MAX_CPUS_PER_CLUSTER; i++) {
		mcucfg_init_archstate(0, i, 1);
		mcucfg_set_bootaddr(0, i, secure_entrypoint);
	}

	if (!check_mcdi_ctl_stat()) {
		HP_SSPM_CTRL = false;
		MCDI_SSPM = false;
	}

	return 0;
}
