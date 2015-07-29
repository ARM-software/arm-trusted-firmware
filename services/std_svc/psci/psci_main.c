/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <runtime_svc.h>
#include <std_svc.h>
#include <string.h>
#include "psci_private.h"

/*******************************************************************************
 * PSCI frontend api for servicing SMCs. Described in the PSCI spec.
 ******************************************************************************/
int psci_cpu_on(u_register_t target_cpu,
		uintptr_t entrypoint,
		u_register_t context_id)

{
	int rc;
	unsigned int end_pwrlvl;
	entry_point_info_t ep;

	/* Determine if the cpu exists of not */
	rc = psci_validate_mpidr(target_cpu);
	if (rc != PSCI_E_SUCCESS)
		return PSCI_E_INVALID_PARAMS;

	/* Validate the entry point and get the entry_point_info */
	rc = psci_validate_entry_point(&ep, entrypoint, context_id);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	/*
	 * To turn this cpu on, specify which power
	 * levels need to be turned on
	 */
	end_pwrlvl = PLAT_MAX_PWR_LVL;
	rc = psci_cpu_on_start(target_cpu,
			    &ep,
			    end_pwrlvl);
	return rc;
}

unsigned int psci_version(void)
{
	return PSCI_MAJOR_VER | PSCI_MINOR_VER;
}

int psci_cpu_suspend(unsigned int power_state,
		     uintptr_t entrypoint,
		     u_register_t context_id)
{
	int rc;
	unsigned int target_pwrlvl, is_power_down_state;
	entry_point_info_t ep;
	psci_power_state_t state_info = { {PSCI_LOCAL_STATE_RUN} };
	plat_local_state_t cpu_pd_state;

	/* Validate the power_state parameter */
	rc = psci_validate_power_state(power_state, &state_info);
	if (rc != PSCI_E_SUCCESS) {
		assert(rc == PSCI_E_INVALID_PARAMS);
		return rc;
	}

	/*
	 * Get the value of the state type bit from the power state parameter.
	 */
	is_power_down_state = psci_get_pstate_type(power_state);

	/* Sanity check the requested suspend levels */
	assert (psci_validate_suspend_req(&state_info, is_power_down_state)
			== PSCI_E_SUCCESS);

	target_pwrlvl = psci_find_target_suspend_lvl(&state_info);

	/* Fast path for CPU standby.*/
	if (is_cpu_standby_req(is_power_down_state, target_pwrlvl)) {
		if  (!psci_plat_pm_ops->cpu_standby)
			return PSCI_E_INVALID_PARAMS;

		/*
		 * Set the state of the CPU power domain to the platform
		 * specific retention state and enter the standby state.
		 */
		cpu_pd_state = state_info.pwr_domain_state[PSCI_CPU_PWR_LVL];
		psci_set_cpu_local_state(cpu_pd_state);
		psci_plat_pm_ops->cpu_standby(cpu_pd_state);

		/* Upon exit from standby, set the state back to RUN. */
		psci_set_cpu_local_state(PSCI_LOCAL_STATE_RUN);

		return PSCI_E_SUCCESS;
	}

	/*
	 * If a power down state has been requested, we need to verify entry
	 * point and program entry information.
	 */
	if (is_power_down_state) {
		rc = psci_validate_entry_point(&ep, entrypoint, context_id);
		if (rc != PSCI_E_SUCCESS)
			return rc;
	}

	/*
	 * Do what is needed to enter the power down state. Upon success,
	 * enter the final wfi which will power down this CPU. This function
	 * might return if the power down was abandoned for any reason, e.g.
	 * arrival of an interrupt
	 */
	psci_cpu_suspend_start(&ep,
			    target_pwrlvl,
			    &state_info,
			    is_power_down_state);

	return PSCI_E_SUCCESS;
}


int psci_system_suspend(uintptr_t entrypoint, u_register_t context_id)
{
	int rc;
	psci_power_state_t state_info;
	entry_point_info_t ep;

	/* Check if the current CPU is the last ON CPU in the system */
	if (!psci_is_last_on_cpu())
		return PSCI_E_DENIED;

	/* Validate the entry point and get the entry_point_info */
	rc = psci_validate_entry_point(&ep, entrypoint, context_id);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	/* Query the psci_power_state for system suspend */
	psci_query_sys_suspend_pwrstate(&state_info);

	/* Ensure that the psci_power_state makes sense */
	assert(psci_find_target_suspend_lvl(&state_info) == PLAT_MAX_PWR_LVL);
	assert(psci_validate_suspend_req(&state_info, PSTATE_TYPE_POWERDOWN)
						== PSCI_E_SUCCESS);
	assert(is_local_state_off(state_info.pwr_domain_state[PLAT_MAX_PWR_LVL]));

	/*
	 * Do what is needed to enter the system suspend state. This function
	 * might return if the power down was abandoned for any reason, e.g.
	 * arrival of an interrupt
	 */
	psci_cpu_suspend_start(&ep,
			    PLAT_MAX_PWR_LVL,
			    &state_info,
			    PSTATE_TYPE_POWERDOWN);

	return PSCI_E_SUCCESS;
}

int psci_cpu_off(void)
{
	int rc;
	unsigned int target_pwrlvl = PLAT_MAX_PWR_LVL;

	/*
	 * Do what is needed to power off this CPU and possible higher power
	 * levels if it able to do so. Upon success, enter the final wfi
	 * which will power down this CPU.
	 */
	rc = psci_do_cpu_off(target_pwrlvl);

	/*
	 * The only error cpu_off can return is E_DENIED. So check if that's
	 * indeed the case.
	 */
	assert (rc == PSCI_E_DENIED);

	return rc;
}

int psci_affinity_info(u_register_t target_affinity,
		       unsigned int lowest_affinity_level)
{
	unsigned int target_idx;

	/* We dont support level higher than PSCI_CPU_PWR_LVL */
	if (lowest_affinity_level > PSCI_CPU_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Calculate the cpu index of the target */
	target_idx = plat_core_pos_by_mpidr(target_affinity);
	if (target_idx == -1)
		return PSCI_E_INVALID_PARAMS;

	return psci_get_aff_info_state_by_idx(target_idx);
}

int psci_migrate(u_register_t target_cpu)
{
	int rc;
	u_register_t resident_cpu_mpidr;

	rc = psci_spd_migrate_info(&resident_cpu_mpidr);
	if (rc != PSCI_TOS_UP_MIG_CAP)
		return (rc == PSCI_TOS_NOT_UP_MIG_CAP) ?
			  PSCI_E_DENIED : PSCI_E_NOT_SUPPORTED;

	/*
	 * Migrate should only be invoked on the CPU where
	 * the Secure OS is resident.
	 */
	if (resident_cpu_mpidr != read_mpidr_el1())
		return PSCI_E_NOT_PRESENT;

	/* Check the validity of the specified target cpu */
	rc = psci_validate_mpidr(target_cpu);
	if (rc != PSCI_E_SUCCESS)
		return PSCI_E_INVALID_PARAMS;

	assert(psci_spd_pm && psci_spd_pm->svc_migrate);

	rc = psci_spd_pm->svc_migrate(read_mpidr_el1(), target_cpu);
	assert(rc == PSCI_E_SUCCESS || rc == PSCI_E_INTERN_FAIL);

	return rc;
}

int psci_migrate_info_type(void)
{
	u_register_t resident_cpu_mpidr;

	return psci_spd_migrate_info(&resident_cpu_mpidr);
}

long psci_migrate_info_up_cpu(void)
{
	u_register_t resident_cpu_mpidr;
	int rc;

	/*
	 * Return value of this depends upon what
	 * psci_spd_migrate_info() returns.
	 */
	rc = psci_spd_migrate_info(&resident_cpu_mpidr);
	if (rc != PSCI_TOS_NOT_UP_MIG_CAP && rc != PSCI_TOS_UP_MIG_CAP)
		return PSCI_E_INVALID_PARAMS;

	return resident_cpu_mpidr;
}

int psci_features(unsigned int psci_fid)
{
	unsigned int local_caps = psci_caps;

	/* Check if it is a 64 bit function */
	if (((psci_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_64)
		local_caps &= PSCI_CAP_64BIT_MASK;

	/* Check for invalid fid */
	if (!(is_std_svc_call(psci_fid) && is_valid_fast_smc(psci_fid)
			&& is_psci_fid(psci_fid)))
		return PSCI_E_NOT_SUPPORTED;


	/* Check if the psci fid is supported or not */
	if (!(local_caps & define_psci_cap(psci_fid)))
		return PSCI_E_NOT_SUPPORTED;

	/* Format the feature flags */
	if (psci_fid == PSCI_CPU_SUSPEND_AARCH32 ||
			psci_fid == PSCI_CPU_SUSPEND_AARCH64) {
		/*
		 * The trusted firmware does not support OS Initiated Mode.
		 */
		return (FF_PSTATE << FF_PSTATE_SHIFT) |
			((!FF_SUPPORTS_OS_INIT_MODE) << FF_MODE_SUPPORT_SHIFT);
	}

	/* Return 0 for all other fid's */
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * PSCI top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t psci_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	if (is_caller_secure(flags))
		SMC_RET1(handle, SMC_UNK);

	/* Check the fid against the capabilities */
	if (!(psci_caps & define_psci_cap(smc_fid)))
		SMC_RET1(handle, SMC_UNK);

	if (((smc_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_32) {
		/* 32-bit PSCI function, clear top parameter bits */

		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;
		x3 = (uint32_t)x3;

		switch (smc_fid) {
		case PSCI_VERSION:
			SMC_RET1(handle, psci_version());

		case PSCI_CPU_OFF:
			SMC_RET1(handle, psci_cpu_off());

		case PSCI_CPU_SUSPEND_AARCH32:
			SMC_RET1(handle, psci_cpu_suspend(x1, x2, x3));

		case PSCI_CPU_ON_AARCH32:
			SMC_RET1(handle, psci_cpu_on(x1, x2, x3));

		case PSCI_AFFINITY_INFO_AARCH32:
			SMC_RET1(handle, psci_affinity_info(x1, x2));

		case PSCI_MIG_AARCH32:
			SMC_RET1(handle, psci_migrate(x1));

		case PSCI_MIG_INFO_TYPE:
			SMC_RET1(handle, psci_migrate_info_type());

		case PSCI_MIG_INFO_UP_CPU_AARCH32:
			SMC_RET1(handle, psci_migrate_info_up_cpu());

		case PSCI_SYSTEM_SUSPEND_AARCH32:
			SMC_RET1(handle, psci_system_suspend(x1, x2));

		case PSCI_SYSTEM_OFF:
			psci_system_off();
			/* We should never return from psci_system_off() */

		case PSCI_SYSTEM_RESET:
			psci_system_reset();
			/* We should never return from psci_system_reset() */

		case PSCI_FEATURES:
			SMC_RET1(handle, psci_features(x1));

		default:
			break;
		}
	} else {
		/* 64-bit PSCI function */

		switch (smc_fid) {
		case PSCI_CPU_SUSPEND_AARCH64:
			SMC_RET1(handle, psci_cpu_suspend(x1, x2, x3));

		case PSCI_CPU_ON_AARCH64:
			SMC_RET1(handle, psci_cpu_on(x1, x2, x3));

		case PSCI_AFFINITY_INFO_AARCH64:
			SMC_RET1(handle, psci_affinity_info(x1, x2));

		case PSCI_MIG_AARCH64:
			SMC_RET1(handle, psci_migrate(x1));

		case PSCI_MIG_INFO_UP_CPU_AARCH64:
			SMC_RET1(handle, psci_migrate_info_up_cpu());

		case PSCI_SYSTEM_SUSPEND_AARCH64:
			SMC_RET1(handle, psci_system_suspend(x1, x2));

		default:
			break;
		}
	}

	WARN("Unimplemented PSCI Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}
