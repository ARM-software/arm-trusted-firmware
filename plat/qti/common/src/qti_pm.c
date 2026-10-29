/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, 2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/sec_core/sec_core.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <platform.h>
#include <platform_def.h>
#include <qti_cpu.h>
#include <qti_plat.h>

#define QTI_LOCAL_PSTATE_MASK		((1 << QTI_LOCAL_PSTATE_WIDTH) - 1)

#if PSCI_OS_INIT_MODE
#define QTI_LAST_AT_PLVL_MASK		(QTI_LOCAL_PSTATE_MASK <<	\
					 (QTI_LOCAL_PSTATE_WIDTH *	\
					  (PLAT_MAX_PWR_LVL + 1)))
#endif

/*******************************************************************************
 * QTI standard platform handler called to check the validity of the power
 * state parameter. The power state parameter has to be a composite power
 * state.
 ******************************************************************************/
int qti_validate_power_state(unsigned int power_state,
			     psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;
	const unsigned int *idle_states = plat_qti_pm_idle_states();

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!idle_states[i]; i++) {
#if PSCI_OS_INIT_MODE
		if ((power_state & ~QTI_LAST_AT_PLVL_MASK) ==
		    idle_states[i])
#else
		if (power_state == idle_states[i])
#endif
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	for (i = QTI_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = state_id &
		    QTI_LOCAL_PSTATE_MASK;
		state_id >>= QTI_LOCAL_PSTATE_WIDTH;
	}
#if PSCI_OS_INIT_MODE
	req_state->last_at_pwrlvl = state_id & QTI_LOCAL_PSTATE_MASK;
#endif

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * PLATFORM FUNCTIONS
 ******************************************************************************/

/**
 * CPU power on function - ideally we want a wrapper since this function is
 * target specific. But to unblock teams.
 */
static int qti_cpu_power_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos_by_mpidr(mpidr);

	/* If not valid mpidr, return error */
	if (core_pos < 0 || core_pos >= PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	return plat_qti_pwr_domain_on(mpidr, core_pos);
}

static bool is_cpu_off(const psci_power_state_t *target_state)
{
	if ((target_state->pwr_domain_state[QTI_PWR_LVL0] ==
	     QTI_LOCAL_STATE_OFF) ||
	    (target_state->pwr_domain_state[QTI_PWR_LVL0] ==
	     QTI_LOCAL_STATE_DEEPOFF)) {
		return true;
	} else {
		return false;
	}
}

static void qti_cpu_power_on_finish(const psci_power_state_t *target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;

	plat_qti_pwr_domain_on_finish(plat_my_core_pos(), pwr_states);

	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_enable();
	}
}

static void qti_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr;

	assert(cpu_state == QTI_LOCAL_STATE_STB);

	/*
	 * Enter standby retention with a WFI, routing non-secure interrupts to
	 * EL3 to wake the CPU, then restore SCR_EL3 so they are handled in the
	 * non-secure world. Route both IRQ and FIQ: GICv3 delivers NS Group 1
	 * interrupts as FIQ, GICv2 as IRQ.
	 */
	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();

	dsb();
	wfi();

	write_scr_el3(scr);
	isb();
}

static void qti_node_power_off(const psci_power_state_t *target_state)
{
	plat_qti_pwr_domain_off((const uint8_t *)target_state->pwr_domain_state);
	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_disable();
	}
}

static void qti_node_suspend(const psci_power_state_t *target_state)
{
	plat_qti_pwr_domain_suspend((const uint8_t *)target_state->pwr_domain_state);
	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_disable();
	}
}

static void qti_node_suspend_finish(const psci_power_state_t *target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	plat_qti_pwr_domain_suspend_finish(pwr_states);
	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_enable();
	}
}

static __dead2 void assert_ps_hold(void)
{
#ifdef QTI_PS_HOLD_REG
	mmio_write_32(QTI_PS_HOLD_REG, 0);
	mdelay(1000);
#endif

	/* Should be dead before reaching this. */
	panic();
}

__dead2 void qti_system_off(void)
{
	qti_pmic_prepare_shutdown();
	assert_ps_hold();
}

__dead2 void qti_system_reset(void)
{
	qti_pmic_prepare_reset();
	assert_ps_hold();
}

void qti_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i;
	unsigned int state_id, power_state;
	const unsigned int *idle_states = plat_qti_pm_idle_states();

	for (i = 0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PSCI_LOCAL_STATE_RUN;
	}

	/*
	 * Find deepest state.
	 * The idle-states array is 0-terminated, so the deepest state is the
	 * last non-zero element, i.e. the one immediately before the terminator.
	 */
	i = 0;
	while (idle_states[i] != 0U) {
		i++;
	}

	/* Need at least one valid state before the terminator. */
	assert(i > 0);
	power_state = idle_states[i - 1];
	state_id = psci_get_pstate_id(power_state);

	i = 0;

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] =
		    state_id & QTI_LOCAL_PSTATE_MASK;
		state_id >>= QTI_LOCAL_PSTATE_WIDTH;
	}

#if PSCI_OS_INIT_MODE
	req_state->last_at_pwrlvl = PLAT_MAX_PWR_LVL;
#endif
}

/*
 * Structure containing platform specific PSCI operations. Common
 * PSCI layer will use this.
 */
const plat_psci_ops_t plat_qti_psci_pm_ops = {
	.pwr_domain_on = qti_cpu_power_on,
	.pwr_domain_on_finish = qti_cpu_power_on_finish,
	.cpu_standby = qti_cpu_standby,
	.pwr_domain_off = qti_node_power_off,
	.pwr_domain_suspend = qti_node_suspend,
	.pwr_domain_suspend_finish = qti_node_suspend_finish,
	.system_off = qti_system_off,
	.system_reset = qti_system_reset,
	.get_node_hw_state = NULL,
	.translate_power_state_by_mpidr = NULL,
	.get_sys_suspend_power_state = qti_get_sys_suspend_power_state,
	.validate_power_state = qti_validate_power_state,
};

/**
 * The QTI Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	int err;

	qti_sec_core_remap((uintptr_t)bl31_warm_entrypoint);

	err = plat_qti_pwr_psci_init((uintptr_t)bl31_warm_entrypoint);
	if (err == PSCI_E_SUCCESS) {
		*psci_ops = &plat_qti_psci_pm_ops;
	}

	return err;
}
