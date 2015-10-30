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
#include <css_pm.h>
#include <plat_arm.h>

/*
 * Custom `validate_power_state` handler for Juno. According to PSCI
 * Specification, interrupts targeted to cores in PSCI CPU SUSPEND should
 * be able to resume it. On Juno, when the system power domain is suspended,
 * the GIC is also powered down. The SCP resumes the final core to be suspend
 * when an external wake-up event is received. But the other cores cannot be
 * woken up by a targeted interrupt, because GIC doesn't forward these
 * interrupts to the SCP. Due to this hardware limitation, we down-grade PSCI
 * CPU SUSPEND requests targeted to the system power domain level
 * to cluster power domain level.
 *
 * The system power domain suspend on Juno is only supported only via
 * PSCI SYSTEM SUSPEND API.
 */
static int juno_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state)
{
	int rc;
	rc = arm_validate_power_state(power_state, req_state);

	/*
	 * Ensure that the system power domain level is never suspended
	 * via PSCI CPU SUSPEND API. Currently system suspend is only
	 * supported via PSCI SYSTEM SUSPEND API.
	 */
	req_state->pwr_domain_state[ARM_PWR_LVL2] = ARM_LOCAL_STATE_RUN;
	return rc;
}

/*******************************************************************************
 * Export the platform handlers via plat_arm_psci_pm_ops. The ARM Standard
 * platform will take care of registering the handlers with PSCI.
 ******************************************************************************/
const plat_psci_ops_t plat_arm_psci_pm_ops = {
	.pwr_domain_on			= css_pwr_domain_on,
	.pwr_domain_on_finish		= css_pwr_domain_on_finish,
	.pwr_domain_off			= css_pwr_domain_off,
	.cpu_standby			= css_cpu_standby,
	.pwr_domain_suspend		= css_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= css_pwr_domain_suspend_finish,
	.system_off			= css_system_off,
	.system_reset			= css_system_reset,
	.validate_power_state		= juno_validate_power_state,
	.validate_ns_entrypoint		= arm_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= css_get_sys_suspend_power_state
};
