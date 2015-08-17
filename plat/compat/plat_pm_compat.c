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
#include <assert.h>
#include <errno.h>
#include <platform.h>
#include <psci.h>

/*
 * The platform hooks exported by the platform using the earlier version of
 * platform interface
 */
const plat_pm_ops_t *pm_ops;

/*
 * The hooks exported by the compatibility layer
 */
static plat_psci_ops_t compat_psci_ops;

/*
 * The secure entry point to be used on warm reset.
 */
static unsigned long secure_entrypoint;

/*
 * This array stores the 'power_state' requests of each CPU during
 * CPU_SUSPEND and SYSTEM_SUSPEND to support querying of state-ID
 * by the platform.
 */
unsigned int psci_power_state_compat[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * The PSCI compatibility helper to parse the power state and populate the
 * 'pwr_domain_state' for each power level. It is assumed that, when in
 * compatibility mode, the PSCI generic layer need to know only whether the
 * affinity level will be OFF or in RETENTION and if the platform supports
 * multiple power down and retention states, it will be taken care within
 * the platform layer.
 ******************************************************************************/
static int parse_power_state(unsigned int power_state,
		    psci_power_state_t *req_state)
{
	int i;
	int pstate = psci_get_pstate_type(power_state);
	int aff_lvl = psci_get_pstate_pwrlvl(power_state);

	if (aff_lvl > PLATFORM_MAX_AFFLVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * Set the CPU local state as retention and ignore the higher
		 * levels. This allows the generic PSCI layer to invoke
		 * plat_psci_ops 'cpu_standby' hook and the compatibility
		 * layer invokes the 'affinst_standby' handler with the
		 * correct power_state parameter thus preserving the correct
		 * behavior.
		 */
		req_state->pwr_domain_state[0] =
					PLAT_MAX_RET_STATE;
	} else {
		for (i = 0; i <= aff_lvl; i++)
			req_state->pwr_domain_state[i] =
					PLAT_MAX_OFF_STATE;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * The PSCI compatibility helper to set the 'power_state' in
 * psci_power_state_compat[] at index corresponding to the current core.
 ******************************************************************************/
static void set_psci_power_state_compat(unsigned int power_state)
{
	unsigned int my_core_pos = plat_my_core_pos();

	psci_power_state_compat[my_core_pos] = power_state;
	flush_dcache_range((uintptr_t) &psci_power_state_compat[my_core_pos],
			sizeof(psci_power_state_compat[my_core_pos]));
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'validate_power_state'
 * hook.
 ******************************************************************************/
static int validate_power_state_compat(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	int rc;
	assert(req_state);

	if (pm_ops->validate_power_state) {
		rc = pm_ops->validate_power_state(power_state);
		if (rc != PSCI_E_SUCCESS)
			return rc;
	}

	/* Store the 'power_state' parameter for the current CPU. */
	set_psci_power_state_compat(power_state);

	return parse_power_state(power_state, req_state);
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t
 * 'get_sys_suspend_power_state' hook.
 ******************************************************************************/
void get_sys_suspend_power_state_compat(psci_power_state_t *req_state)
{
	unsigned int power_state;
	assert(req_state);

	power_state = pm_ops->get_sys_suspend_power_state();

	/* Store the 'power_state' parameter for the current CPU. */
	set_psci_power_state_compat(power_state);

	if (parse_power_state(power_state, req_state) != PSCI_E_SUCCESS)
		assert(0);
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'validate_ns_entrypoint'
 * hook.
 ******************************************************************************/
static int validate_ns_entrypoint_compat(uintptr_t ns_entrypoint)
{
	return pm_ops->validate_ns_entrypoint(ns_entrypoint);
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'affinst_standby' hook.
 ******************************************************************************/
static void cpu_standby_compat(plat_local_state_t cpu_state)
{
	unsigned int powerstate = psci_get_suspend_powerstate();

	assert(powerstate != PSCI_INVALID_DATA);

	pm_ops->affinst_standby(powerstate);
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'affinst_on' hook.
 ******************************************************************************/
static int pwr_domain_on_compat(u_register_t mpidr)
{
	int level, rc;

	/*
	 * The new PSCI framework does not hold the locks for higher level
	 * power domain nodes when this hook is invoked. Hence figuring out the
	 * target state of the parent power domains does not make much sense.
	 * Hence we hard-code the state as PSCI_STATE_OFF for all the levels.
	 * We expect the platform to perform the necessary CPU_ON operations
	 * when the 'affinst_on' is invoked only for level 0.
	 */
	for (level = PLATFORM_MAX_AFFLVL; level >= 0; level--) {
		rc = pm_ops->affinst_on((unsigned long)mpidr, secure_entrypoint,
					level, PSCI_STATE_OFF);
		if (rc != PSCI_E_SUCCESS)
			break;
	}

	return rc;
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'affinst_off' hook.
 ******************************************************************************/
static void pwr_domain_off_compat(const psci_power_state_t *target_state)
{
	int level;
	unsigned int plat_state;

	for (level = 0; level <= PLATFORM_MAX_AFFLVL; level++) {
		plat_state = (is_local_state_run(
				target_state->pwr_domain_state[level]) ?
				PSCI_STATE_ON : PSCI_STATE_OFF);
		pm_ops->affinst_off(level, plat_state);
	}
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'affinst_suspend' hook.
 ******************************************************************************/
static void pwr_domain_suspend_compat(const psci_power_state_t *target_state)
{
	int level;
	unsigned int plat_state;

	for (level = 0; level <= psci_get_suspend_afflvl(); level++) {
		plat_state = (is_local_state_run(
				target_state->pwr_domain_state[level]) ?
				PSCI_STATE_ON : PSCI_STATE_OFF);
		pm_ops->affinst_suspend(secure_entrypoint, level, plat_state);
	}
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'affinst_on_finish'
 * hook.
 ******************************************************************************/
static void pwr_domain_on_finish_compat(const psci_power_state_t *target_state)
{
	int level;
	unsigned int plat_state;

	for (level = PLATFORM_MAX_AFFLVL; level >= 0; level--) {
		plat_state = (is_local_state_run(
				target_state->pwr_domain_state[level]) ?
				PSCI_STATE_ON : PSCI_STATE_OFF);
		pm_ops->affinst_on_finish(level, plat_state);
	}
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t
 * 'affinst_suspend_finish' hook.
 ******************************************************************************/
static void pwr_domain_suspend_finish_compat(
				const psci_power_state_t *target_state)
{
	int level;
	unsigned int plat_state;

	for (level = psci_get_suspend_afflvl(); level >= 0; level--) {
		plat_state = (is_local_state_run(
				target_state->pwr_domain_state[level]) ?
				PSCI_STATE_ON : PSCI_STATE_OFF);
		pm_ops->affinst_suspend_finish(level, plat_state);
	}
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'system_off' hook.
 ******************************************************************************/
static void __dead2 system_off_compat(void)
{
	pm_ops->system_off();
}

/*******************************************************************************
 * The PSCI compatibility helper for plat_pm_ops_t 'system_reset' hook.
 ******************************************************************************/
static void __dead2 system_reset_compat(void)
{
	pm_ops->system_reset();
}

/*******************************************************************************
 * Export the compatibility compat_psci_ops. The assumption made is that the
 * power domains correspond to affinity instances on the platform.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
				const plat_psci_ops_t **psci_ops)
{
	platform_setup_pm(&pm_ops);

	secure_entrypoint = (unsigned long) sec_entrypoint;

	/*
	 * It is compulsory for the platform ports using the new porting
	 * interface to export a hook to validate the power state parameter
	 */
	compat_psci_ops.validate_power_state = validate_power_state_compat;

	/*
	 * Populate the compatibility plat_psci_ops_t hooks if available
	 */
	if (pm_ops->validate_ns_entrypoint)
		compat_psci_ops.validate_ns_entrypoint =
				validate_ns_entrypoint_compat;

	if (pm_ops->affinst_standby)
		compat_psci_ops.cpu_standby = cpu_standby_compat;

	if (pm_ops->affinst_on)
		compat_psci_ops.pwr_domain_on = pwr_domain_on_compat;

	if (pm_ops->affinst_off)
		compat_psci_ops.pwr_domain_off = pwr_domain_off_compat;

	if (pm_ops->affinst_suspend)
		compat_psci_ops.pwr_domain_suspend = pwr_domain_suspend_compat;

	if (pm_ops->affinst_on_finish)
		compat_psci_ops.pwr_domain_on_finish =
				pwr_domain_on_finish_compat;

	if (pm_ops->affinst_suspend_finish)
		compat_psci_ops.pwr_domain_suspend_finish =
				pwr_domain_suspend_finish_compat;

	if (pm_ops->system_off)
		compat_psci_ops.system_off = system_off_compat;

	if (pm_ops->system_reset)
		compat_psci_ops.system_reset = system_reset_compat;

	if (pm_ops->get_sys_suspend_power_state)
		compat_psci_ops.get_sys_suspend_power_state =
				get_sys_suspend_power_state_compat;

	*psci_ops = &compat_psci_ops;
	return 0;
}
