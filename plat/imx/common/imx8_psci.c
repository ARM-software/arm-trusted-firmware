/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>

#include <plat_imx8.h>
#include <sci/sci.h>

void __dead2 imx_system_off(void)
{
	sc_pm_set_sys_power_mode(ipc_handle, SC_PM_PW_MODE_OFF);
	wfi();
	ERROR("power off failed.\n");
	panic();
}

void __dead2 imx_system_reset(void)
{
	sc_pm_reset(ipc_handle, SC_PM_RESET_TYPE_BOARD);
	wfi();
	ERROR("system reset failed.\n");
	panic();
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	/* TODO */
	return PSCI_E_INVALID_PARAMS;
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	/* CPU & cluster off, system in retention */
	for (i = MPIDR_AFFLVL0; i < PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] = PLAT_MAX_RET_STATE;
}

