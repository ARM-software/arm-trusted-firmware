/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <plat_imx8.h>
#include <pwr_ctrl.h>

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

/* platform secure warm boot entry */
static uintptr_t secure_entrypoint;

static bool boot_stage = true;

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	if (pwr_type == PSTATE_TYPE_POWERDOWN && state_id == 0x33) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_set_cpu_boot_entry(unsigned int core_id, uint64_t boot_entry)
{
	/* set the cpu core reset entry: BLK_CTRL_S */
	mmio_write_32(BLK_CTRL_S_BASE + CA55_RVBADDR0_L + core_id * 8, boot_entry >> 2);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;

	core_id = MPIDR_AFFLVL1_VAL(mpidr);

	imx_set_cpu_boot_entry(core_id, secure_entrypoint);

	/*
	 * When the core is first time boot up, the core is already ON after SoC POR,
	 * So 'SW_WAKEUP' can not work, so need to toggle core's reset then release
	 * the core from cpu_wait.
	 */
	if (boot_stage) {
		/* assert CPU core SW reset */
		mmio_clrbits_32(SRC_SLICE(SRC_A55C0 + core_id) + 0x24, BIT(2) | BIT(0));
		/* deassert CPU core SW reset */
		mmio_setbits_32(SRC_SLICE(SRC_A55C0 + core_id) + 0x24, BIT(2) | BIT(0));
		/* release the cpuwait to kick the cpu */
		mmio_clrbits_32(BLK_CTRL_S_BASE + CA55_CPUWAIT, BIT(core_id));
	} else {
		/* assert the CMC MISC SW WAKEUP BIT to kick the offline core */
		gpc_assert_sw_wakeup(CPU_A55C0 + core_id);
	}

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();

	/* below config is ok both for boot & hotplug */
	/* clear the CPU power mode */
	gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_RUN);
	/* clear the SW wakeup */
	gpc_deassert_sw_wakeup(CPU_A55C0 + core_id);
	/* switch to GIC wakeup source */
	gpc_select_wakeup_gic(CPU_A55C0 + core_id);

	if (boot_stage) {
		/* SRC config */
		/* config the MEM LPM */
		src_mem_lpm_en(SRC_A55P0_MEM + core_id, MEM_OFF);
		/* LPM config to only ON in run mode to its domain */
		src_mix_set_lpm(SRC_A55C0 + core_id, core_id, CM_MODE_WAIT);
		/* white list config, only enable its own domain */
		src_authen_config(SRC_A55C0 + core_id, 1 << core_id, 0x1);

		boot_stage = false;
	}
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int i;

	plat_gic_cpuif_disable();
	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/*
	 * mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core
	 * as we need to use SW_WAKEUP for hotplug purpose
	 */
	for (i = 0U; i < IMR_NUM; i++) {
		gpc_set_irq_mask(CPU_A55C0 + core_id, i, 0xffffffff);
	}
	/* switch to GPC wakeup source */
	gpc_select_wakeup_raw_irq(CPU_A55C0 + core_id);
	/* config the target mode to suspend */
	gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_SUSPEND);
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		imx_set_cpu_boot_entry(core_id, secure_entrypoint);
		/* config the target mode to WAIT */
		gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_WAIT);
	}

	/* do cluster level config */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* config the A55 cluster target mode to WAIT */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_WAIT);

		/* config DSU for cluster power down with L3 MEM RET */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF | BIT(1));
		}
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* cluster level */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* set the cluster's target mode to RUN */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_RUN);
	}

	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* set A55 CORE's power mode to RUN */
		gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_RUN);
		plat_gic_cpuif_enable();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}

	SYSTEM_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
}

void __dead2 imx_system_reset(void)
{
	mmio_write_32(WDOG3_BASE + WDOG_CNT, 0xd928c520);
	while ((mmio_read_32(WDOG3_BASE + WDOG_CS) & WDOG_CS_ULK) == 0U) {
		;
	}

	mmio_write_32(WDOG3_BASE + WDOG_TOVAL, 0x10);
	mmio_write_32(WDOG3_BASE + WDOG_CS, 0x21e3);

	while (1) {
		wfi();
	}
}

void __dead2 imx_system_off(void)
{
	mmio_setbits_32(BBNSM_BASE + BBNSM_CTRL, BBNSM_DP_EN | BBNSM_TOSP);

	while (1) {
		wfi();
	}
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_suspend = imx_pwr_domain_suspend,
	.pwr_domain_suspend_finish = imx_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0, sec_entrypoint);

	pwr_sys_init();

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
