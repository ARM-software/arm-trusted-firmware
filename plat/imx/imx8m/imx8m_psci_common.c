/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
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

#include <gpc.h>
#include <imx8m_psci.h>
#include <plat_imx8.h>

/*
 * below callback functions need to be override by i.mx8mq,
 * for other i.mx8m soc, if no special requirement,
 * reuse below ones.
 */
#pragma weak imx_validate_power_state
#pragma weak imx_domain_suspend
#pragma weak imx_domain_suspend_finish
#pragma weak imx_get_sys_suspend_power_state

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;
	uint64_t base_addr = BL31_BASE;

	core_id = MPIDR_AFFLVL0_VAL(mpidr);

	imx_set_cpu_secure_entry(core_id, base_addr);
	imx_set_cpu_pwr_on(core_id);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	imx_set_cpu_pwr_off(core_id);
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
		CLUSTER_PWR_STATE(req_state) = PLAT_WAIT_RET_STATE;
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
		plat_gic_cpuif_disable();
		imx_set_cpu_secure_entry(core_id, base_addr);
		imx_set_cpu_lpm(core_id, true);
	} else {
		dsb();
		write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
		isb();
	}

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state)))
		imx_set_cluster_powerdown(core_id, CLUSTER_PWR_STATE(target_state));

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state)))
		imx_set_sys_lpm(core_id, true);
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state)))
		imx_set_sys_lpm(core_id, false);

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		imx_clear_rbc_count();
		imx_set_cluster_powerdown(core_id, PSCI_LOCAL_STATE_RUN);
	}

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_set_cpu_lpm(core_id, false);
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~SCR_FIQ_BIT));
		isb();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_STOP_OFF_STATE;
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
	/*
	 * before enter WAIT or STOP mode with PLAT(SCU) power down,
	 * rbc count need to be enabled to make sure PLAT is
	 * power down successfully even if the the wakeup IRQ is pending
	 * early before the power down sequence. the RBC counter is
	 * drived by the 32K OSC, so delay 30us to make sure the counter
	 * is really running.
	 */
	if (is_local_state_off(CLUSTER_PWR_STATE(target_state))) {
		imx_set_rbc_count();
		udelay(30);
	}

	while (1)
		wfi();
}
