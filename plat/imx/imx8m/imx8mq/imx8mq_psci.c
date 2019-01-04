/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <gpc.h>
#include <plat_imx8.h>

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;
	uint64_t base_addr = BL31_BASE;

	core_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* set the secure entrypoint */
	imx_set_cpu_secure_entry(core_id, base_addr);
	/* power up the core */
	imx_set_cpu_pwr_on(core_id);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* program the GIC per cpu dist and rdist interface */
	plat_gic_pcpu_init();
	/* enable the GICv3 cpu interface */
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* disable the GIC cpu interface first */
	plat_gic_cpuif_disable();
	/* config the core for power down */
	imx_set_cpu_pwr_off(core_id);
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

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

void imx_cpu_standby(plat_local_state_t cpu_state)
{
	dsb();
	write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
	isb();

	wfi();

	write_scr_el3(read_scr_el3() & (~SCR_FIQ_BIT));
	isb();
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t base_addr = BL31_BASE;
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* disable the cpu interface */
		plat_gic_cpuif_disable();
		imx_set_cpu_secure_entry(core_id, base_addr);
		imx_set_cpu_lpm(core_id, true);
	} else {
		dsb();
		write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
		isb();
	}

	if (is_local_state_off(CLUSTER_PWR_STATE(target_state)))
		imx_set_cluster_powerdown(core_id, true);
	else
		imx_set_cluster_standby(true);

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		imx_set_sys_lpm(true);
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* check the system level status */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		imx_set_sys_lpm(false);
		imx_clear_rbc_count();
	}

	/* check the cluster level power status */
	if (is_local_state_off(CLUSTER_PWR_STATE(target_state)))
		imx_set_cluster_powerdown(core_id, false);
	else
		imx_set_cluster_standby(false);

	/* check the core level power status */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* clear the core lpm setting */
		imx_set_cpu_lpm(core_id, false);
		/* enable the gic cpu interface */
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~0x4));
		isb();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i < PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_STOP_OFF_STATE;

	req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] = PLAT_MAX_RET_STATE;
}

void __dead2 imx_system_reset(void)
{
	uintptr_t wdog_base = IMX_WDOG_BASE;
	unsigned int val;

	/* WDOG_B reset */
	val = mmio_read_16(wdog_base);
#ifdef IMX_WDOG_B_RESET
	val = (val & 0x00FF) | WDOG_WCR_WDZST | WDOG_WCR_WDE |
		WDOG_WCR_WDT | WDOG_WCR_SRS;
#else
	val = (val & 0x00FF) | WDOG_WCR_WDZST | WDOG_WCR_SRS;
#endif
	mmio_write_16(wdog_base, val);

	mmio_write_16(wdog_base + WDOG_WSR, 0x5555);
	mmio_write_16(wdog_base + WDOG_WSR, 0xaaaa);
	while (1)
		;
}



void __dead2 imx_system_off(void)
{
	mmio_write_32(IMX_SNVS_BASE + SNVS_LPCR, SNVS_LPCR_SRTC_ENV |
			SNVS_LPCR_DP_EN | SNVS_LPCR_TOP);

	while (1)
		;
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	if (is_local_state_off(CLUSTER_PWR_STATE(target_state)))
		imx_set_rbc_count();

	while (1)
		wfi();
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.cpu_standby = imx_cpu_standby,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	/* sec_entrypoint is used for warm reset */
	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
