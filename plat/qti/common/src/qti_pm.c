/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#include <platform.h>
#include "psci.h"
#include <qtiseclib_interface.h>
#include "debug.h"
#include "assert.h"

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
	if (core_pos < 0 || core_pos >= QTISECLIB_PLAT_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	return qtiseclib_psci_node_power_on(mpidr);
}

static void qti_cpu_power_on_finish(const psci_power_state_t * target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	qtiseclib_psci_node_on_finish(pwr_states);
}

static void qti_cpu_standby(plat_local_state_t cpu_state)
{
	qtiseclib_psci_cpu_standby((uint8_t) cpu_state);
}

static void qti_node_power_off(const psci_power_state_t * target_state)
{
	qtiseclib_psci_node_power_off((const uint8_t *)
				       target_state->pwr_domain_state);
}

static void qti_node_suspend(const psci_power_state_t * target_state)
{
	qtiseclib_psci_node_suspend((const uint8_t *)target_state->pwr_domain_state);
}

static void qti_node_suspend_finish(const psci_power_state_t * target_state)
{
	const uint8_t *pwr_states =
	    (const uint8_t *)target_state->pwr_domain_state;
	qtiseclib_psci_node_suspend_finish(pwr_states);
}

__dead2 void qti_domain_power_down_wfi(const psci_power_state_t * target_state)
{
	/* For now just do WFI - add any target specific handling if needed */

	__asm volatile ("dsb sy \n"
			"wfi");

	/* We should never reach here */
	ERROR("PSCI: WFI fell through during power down (%d)",
	      plat_my_core_pos());
	panic();
}

void qti_get_sys_suspend_power_state(psci_power_state_t * req_state)
{
	int i;

	for (i = 0; i <= PSCI_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

int qti_validate_power_state(unsigned int power_state,
			     psci_power_state_t * req_state)
{
	assert(req_state);
	return qtiseclib_psci_validate_power_state(power_state,
					      req_state->pwr_domain_state);
}

/* Structure containing platform specific PSCI operations. Common
 * PSCI layer will use this. */
const plat_psci_ops_t plat_qti_psci_pm_ops = {
	.pwr_domain_on = qti_cpu_power_on,
	.pwr_domain_on_finish = qti_cpu_power_on_finish,
	.cpu_standby = qti_cpu_standby,
	.pwr_domain_off = qti_node_power_off,
	.pwr_domain_suspend = qti_node_suspend,
	.pwr_domain_suspend_finish = qti_node_suspend_finish,
	.pwr_domain_pwr_down_wfi = qti_domain_power_down_wfi,
	.get_node_hw_state = NULL,
	.translate_power_state_by_mpidr = NULL,
	.get_sys_suspend_power_state = qti_get_sys_suspend_power_state,
	.validate_power_state = qti_validate_power_state,
};

/**
 * The ARM Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t ** psci_ops)
{
	int err;

	err = qtiseclib_psci_init();
	if (err == PSCI_E_SUCCESS) {
		*psci_ops = &plat_qti_psci_pm_ops;
	}

	return err;
}
