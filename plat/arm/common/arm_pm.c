/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <arm_def.h>
#include <arm_gic.h>
#include <assert.h>
#include <console.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform_def.h>
#include <psci.h>

/* Standard ARM platforms are expected to export plat_arm_psci_pm_ops */
extern const plat_psci_ops_t plat_arm_psci_pm_ops;

#if ARM_RECOM_STATE_ID_ENC
extern unsigned int arm_pm_idle_states[];
#endif /* __ARM_RECOM_STATE_ID_ENC__ */

#if !ARM_RECOM_STATE_ID_ENC
/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power state
 * parameter.
 ******************************************************************************/
int arm_validate_power_state(unsigned int power_state,
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
		if (pwr_lvl != ARM_PWR_LVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[ARM_PWR_LVL0] =
					ARM_LOCAL_STATE_RET;
	} else {
		for (i = ARM_PWR_LVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					ARM_LOCAL_STATE_OFF;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

#else
/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power
 * state parameter. The power state parameter has to be a composite power
 * state.
 ******************************************************************************/
int arm_validate_power_state(unsigned int power_state,
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
	for (i = 0; !!arm_pm_idle_states[i]; i++) {
		if (power_state == arm_pm_idle_states[i])
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!arm_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
						ARM_LOCAL_PSTATE_MASK;
		state_id >>= ARM_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}
#endif /* __ARM_RECOM_STATE_ID_ENC__ */

/*******************************************************************************
 * ARM standard platform handler called to check the validity of the non secure
 * entrypoint.
 ******************************************************************************/
int arm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= ARM_NS_DRAM1_BASE) && (entrypoint <
			(ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE)))
		return PSCI_E_SUCCESS;
	if ((entrypoint >= ARM_DRAM2_BASE) && (entrypoint <
			(ARM_DRAM2_BASE + ARM_DRAM2_SIZE)))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

/******************************************************************************
 * Helper function to resume the platform from system suspend. Reinitialize
 * the system components which are not in the Always ON power domain.
 * TODO: Unify the platform setup when waking up from cold boot and system
 * resume in arm_bl31_platform_setup().
 *****************************************************************************/
void arm_system_pwr_domain_resume(void)
{
	console_init(PLAT_ARM_BL31_RUN_UART_BASE, PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ,
						ARM_CONSOLE_BAUDRATE);

	/* Assert system power domain is available on the platform */
	assert(PLAT_MAX_PWR_LVL >= ARM_PWR_LVL2);

	/*
	 * TODO: On GICv3 systems, figure out whether the core that wakes up
	 * first from system suspend need to initialize the re-distributor
	 * interface of all the other suspended cores.
	 */
	plat_arm_gic_init();
	plat_arm_security_setup();
	arm_configure_sys_timer();
}

/*******************************************************************************
 * Private function to program the mailbox for a cpu before it is released
 * from reset. This function assumes that the Trusted mail box base is within
 * the ARM_SHARED_RAM region
 ******************************************************************************/
void arm_program_trusted_mailbox(uintptr_t address)
{
	uintptr_t *mailbox = (void *) PLAT_ARM_TRUSTED_MAILBOX_BASE;

	*mailbox = address;

	/*
	 * Ensure that the PLAT_ARM_TRUSTED_MAILBOX_BASE is within
	 * ARM_SHARED_RAM region.
	 */
	assert((PLAT_ARM_TRUSTED_MAILBOX_BASE >= ARM_SHARED_RAM_BASE) &&
		((PLAT_ARM_TRUSTED_MAILBOX_BASE + sizeof(*mailbox)) <= \
				(ARM_SHARED_RAM_BASE + ARM_SHARED_RAM_SIZE)));
}

/*******************************************************************************
 * The ARM Standard platform definition of platform porting API
 * `plat_setup_psci_ops`.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_arm_psci_pm_ops;

	/* Setup mailbox with entry point. */
	arm_program_trusted_mailbox(sec_entrypoint);
	return 0;
}
