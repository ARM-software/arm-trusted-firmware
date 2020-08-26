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
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <platform.h>
#include <platform_def.h>
#include <qti_cpu.h>
#include <qti_plat.h>
#include <qtiseclib_cb_interface.h>
#include <qtiseclib_defs_plat.h>
#include <qtiseclib_interface.h>

#define QTI_LOCAL_PSTATE_WIDTH		4
#define QTI_LOCAL_PSTATE_MASK		((1 << QTI_LOCAL_PSTATE_WIDTH) - 1)

/* Make composite power state parameter till level 0 */
#define qti_make_pwrstate_lvl0(lvl0_state, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))

/* Make composite power state parameter till level 1 */
#define qti_make_pwrstate_lvl1(lvl1_state, lvl0_state, type) \
		(((lvl1_state) << QTI_LOCAL_PSTATE_WIDTH) | \
		qti_make_pwrstate_lvl0(lvl0_state, type))

/* Make composite power state parameter till level 2 */
#define qti_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, type) \
		(((lvl2_state) << (QTI_LOCAL_PSTATE_WIDTH * 2)) | \
		qti_make_pwrstate_lvl1(lvl1_state, lvl0_state, type))

/* Make composite power state parameter till level 3 */
#define qti_make_pwrstate_lvl3(lvl3_state, lvl2_state, lvl1_state, lvl0_state, type) \
		(((lvl3_state) << (QTI_LOCAL_PSTATE_WIDTH * 3)) | \
		qti_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, type))

/* QTI_CORE_PWRDN_EN_MASK happens to be same across all CPUs */
#define QTI_CORE_PWRDN_EN_MASK		1

/* cpu power control happens to be same across all CPUs */
_DEFINE_SYSREG_WRITE_FUNC(cpu_pwrctrl_val, S3_0_C15_C2_7)
_DEFINE_SYSREG_READ_FUNC(cpu_pwrctrl_val, S3_0_C15_C2_7)

const unsigned int qti_pm_idle_states[] = {
	qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_OFF,
			       PSTATE_TYPE_POWERDOWN),
	qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	qti_make_pwrstate_lvl1(QTI_LOCAL_STATE_DEEPOFF,
			       QTI_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	qti_make_pwrstate_lvl2(QTI_LOCAL_STATE_OFF,
			       QTI_LOCAL_STATE_DEEPOFF,
			       QTI_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	qti_make_pwrstate_lvl3(QTI_LOCAL_STATE_OFF,
			       QTI_LOCAL_STATE_DEEPOFF,
			       QTI_LOCAL_STATE_DEEPOFF,
			       QTI_LOCAL_STATE_DEEPOFF,
			       PSTATE_TYPE_POWERDOWN),
	0,
};

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

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!qti_pm_idle_states[i]; i++) {
		if (power_state == qti_pm_idle_states[i])
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!qti_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
		    QTI_LOCAL_PSTATE_MASK;
		state_id >>= QTI_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * PLATFORM FUNCTIONS
 ******************************************************************************/

static void qti_set_cpupwrctlr_val(void)
{
	unsigned long val;

	val = read_cpu_pwrctrl_val();
	val |= QTI_CORE_PWRDN_EN_MASK;
	write_cpu_pwrctrl_val(val);

	isb();
}

/**
 * CPU power on function - ideally we want a wrapper since this function is
 * target specific. But to unblock teams.
 */
static int qti_cpu_power_on(u_register_t mpidr)
{
	int core_pos = plat_core_pos_by_mpidr(mpidr);

	/* If not valid mpidr, return error */
	if (core_pos < 0 || core_pos >= QTISECLIB_PLAT_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	return qtiseclib_psci_node_power_on(mpidr);
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
	qtiseclib_psci_node_on_finish(pwr_states);

	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_enable();
	}
}

static void qti_cpu_standby(plat_local_state_t cpu_state)
{
}

static void qti_node_power_off(const psci_power_state_t *target_state)
{
	qtiseclib_psci_node_power_off((const uint8_t *)
				      target_state->pwr_domain_state);
	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_disable();
		qti_set_cpupwrctlr_val();
	}
}

static void qti_node_suspend(const psci_power_state_t *target_state)
{
	qtiseclib_psci_node_suspend((const uint8_t *)target_state->
				    pwr_domain_state);
	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_disable();
		qti_set_cpupwrctlr_val();
	}
}

static void qti_node_suspend_finish(const psci_power_state_t *target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	qtiseclib_psci_node_suspend_finish(pwr_states);
	if (is_cpu_off(target_state)) {
		plat_qti_gic_cpuif_enable();
	}
}

__dead2 void qti_domain_power_down_wfi(const psci_power_state_t *target_state)
{

	/* For now just do WFI - add any target specific handling if needed */
	psci_power_down_wfi();
	/* We should never reach here */
}

static __dead2 void assert_ps_hold(void)
{
	mmio_write_32(QTI_PS_HOLD_REG, 0);
	mdelay(1000);

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
	int i = 0;
	unsigned int state_id, power_state;
	int size = ARRAY_SIZE(qti_pm_idle_states);

	/*
	 * Find deepest state.
	 * The arm_pm_idle_states[] array has last element by default 0,
	 * so the real deepest state is second last element of that array.
	 */
	power_state = qti_pm_idle_states[size - 2];
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] =
		    state_id & QTI_LOCAL_PSTATE_MASK;
		state_id >>= QTI_LOCAL_PSTATE_WIDTH;
	}
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
	.pwr_domain_pwr_down_wfi = qti_domain_power_down_wfi,
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

	err = qtiseclib_psci_init((uintptr_t)bl31_warm_entrypoint);
	if (err == PSCI_E_SUCCESS) {
		*psci_ops = &plat_qti_psci_pm_ops;
	}

	return err;
}
