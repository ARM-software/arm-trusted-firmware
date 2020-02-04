/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Allow ARM Standard platforms to override these functions */
#pragma weak plat_arm_program_trusted_mailbox

#if !ARM_RECOM_STATE_ID_ENC
/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power state
 * parameter.
 ******************************************************************************/
int arm_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int i;

	assert(req_state != NULL);

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
	if (psci_get_pstate_id(power_state) != 0U)
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

	assert(req_state != NULL);

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
 * entrypoint. Returns 0 if the entrypoint is valid, or -1 otherwise.
 ******************************************************************************/
int arm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= ARM_NS_DRAM1_BASE) && (entrypoint <
			(ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE))) {
		return 0;
	}
#ifdef __aarch64__
	if ((entrypoint >= ARM_DRAM2_BASE) && (entrypoint <
			(ARM_DRAM2_BASE + ARM_DRAM2_SIZE))) {
		return 0;
	}
#endif

	return -1;
}

int arm_validate_psci_entrypoint(uintptr_t entrypoint)
{
	return (arm_validate_ns_entrypoint(entrypoint) == 0) ? PSCI_E_SUCCESS :
		PSCI_E_INVALID_ADDRESS;
}

/******************************************************************************
 * Helper function to save the platform state before a system suspend. Save the
 * state of the system components which are not in the Always ON power domain.
 *****************************************************************************/
void arm_system_pwr_domain_save(void)
{
	/* Assert system power domain is available on the platform */
	assert(PLAT_MAX_PWR_LVL >= ARM_PWR_LVL2);

	plat_arm_gic_save();

	/*
	 * Unregister console now so that it is not registered for a second
	 * time during resume.
	 */
	arm_console_runtime_end();

	/*
	 * All the other peripheral which are configured by ARM TF are
	 * re-initialized on resume from system suspend. Hence we
	 * don't save their state here.
	 */
}

/******************************************************************************
 * Helper function to resume the platform from system suspend. Reinitialize
 * the system components which are not in the Always ON power domain.
 * TODO: Unify the platform setup when waking up from cold boot and system
 * resume in arm_bl31_platform_setup().
 *****************************************************************************/
void arm_system_pwr_domain_resume(void)
{
	/* Initialize the console */
	arm_console_runtime_init();

	/* Assert system power domain is available on the platform */
	assert(PLAT_MAX_PWR_LVL >= ARM_PWR_LVL2);

	plat_arm_gic_resume();

	plat_arm_security_setup();
	arm_configure_sys_timer();
}

/*******************************************************************************
 * ARM platform function to program the mailbox for a cpu before it is released
 * from reset. This function assumes that the Trusted mail box base is within
 * the ARM_SHARED_RAM region
 ******************************************************************************/
void plat_arm_program_trusted_mailbox(uintptr_t address)
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
int __init plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	*psci_ops = plat_arm_psci_override_pm_ops(&plat_arm_psci_pm_ops);

	/* Setup mailbox with entry point. */
	plat_arm_program_trusted_mailbox(sec_entrypoint);
	return 0;
}
