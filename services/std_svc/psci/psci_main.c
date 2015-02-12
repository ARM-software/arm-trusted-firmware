/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <runtime_svc.h>
#include <std_svc.h>
#include <debug.h>
#include "psci_private.h"

/*******************************************************************************
 * PSCI frontend api for servicing SMCs. Described in the PSCI spec.
 ******************************************************************************/
int psci_cpu_on(unsigned long target_cpu,
		unsigned long entrypoint,
		unsigned long context_id)

{
	int rc;
	unsigned int start_afflvl, end_afflvl;
	entry_point_info_t ep;

	/* Determine if the cpu exists of not */
	rc = psci_validate_mpidr(target_cpu, MPIDR_AFFLVL0);
	if (rc != PSCI_E_SUCCESS) {
		return PSCI_E_INVALID_PARAMS;
	}

	/* Validate the entrypoint using platform pm_ops */
	if (psci_plat_pm_ops->validate_ns_entrypoint) {
		rc = psci_plat_pm_ops->validate_ns_entrypoint(entrypoint);
		if (rc != PSCI_E_SUCCESS) {
			assert(rc == PSCI_E_INVALID_PARAMS);
			return PSCI_E_INVALID_PARAMS;
		}
	}

	/*
	 * Verify and derive the re-entry information for
	 * the non-secure world from the non-secure state from
	 * where this call originated.
	 */
	rc = psci_get_ns_ep_info(&ep, entrypoint, context_id);
	if (rc != PSCI_E_SUCCESS)
		return rc;


	/*
	 * To turn this cpu on, specify which affinity
	 * levels need to be turned on
	 */
	start_afflvl = MPIDR_AFFLVL0;
	end_afflvl = PLATFORM_MAX_AFFLVL;
	rc = psci_afflvl_on(target_cpu,
			    &ep,
			    start_afflvl,
			    end_afflvl);

	return rc;
}

unsigned int psci_version(void)
{
	return PSCI_MAJOR_VER | PSCI_MINOR_VER;
}

int psci_cpu_suspend(unsigned int power_state,
		     unsigned long entrypoint,
		     unsigned long context_id)
{
	int rc;
	unsigned int target_afflvl, pstate_type;
	entry_point_info_t ep;

	/* Check SBZ bits in power state are zero */
	if (psci_validate_power_state(power_state))
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	target_afflvl = psci_get_pstate_afflvl(power_state);
	if (target_afflvl > PLATFORM_MAX_AFFLVL)
		return PSCI_E_INVALID_PARAMS;

	/* Validate the power_state using platform pm_ops */
	if (psci_plat_pm_ops->validate_power_state) {
		rc = psci_plat_pm_ops->validate_power_state(power_state);
		if (rc != PSCI_E_SUCCESS) {
			assert(rc == PSCI_E_INVALID_PARAMS);
			return PSCI_E_INVALID_PARAMS;
		}
	}

	/* Validate the entrypoint using platform pm_ops */
	if (psci_plat_pm_ops->validate_ns_entrypoint) {
		rc = psci_plat_pm_ops->validate_ns_entrypoint(entrypoint);
		if (rc != PSCI_E_SUCCESS) {
			assert(rc == PSCI_E_INVALID_PARAMS);
			return PSCI_E_INVALID_PARAMS;
		}
	}

	/* Determine the 'state type' in the 'power_state' parameter */
	pstate_type = psci_get_pstate_type(power_state);

	/*
	 * Ensure that we have a platform specific handler for entering
	 * a standby state.
	 */
	if (pstate_type == PSTATE_TYPE_STANDBY) {
		if  (!psci_plat_pm_ops->affinst_standby)
			return PSCI_E_INVALID_PARAMS;

		psci_plat_pm_ops->affinst_standby(power_state);
		return PSCI_E_SUCCESS;
	}

	/*
	 * Verify and derive the re-entry information for
	 * the non-secure world from the non-secure state from
	 * where this call originated.
	 */
	rc = psci_get_ns_ep_info(&ep, entrypoint, context_id);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	/* Save PSCI power state parameter for the core in suspend context */
	psci_set_suspend_power_state(power_state);

	/*
	 * Do what is needed to enter the power down state. Upon success,
	 * enter the final wfi which will power down this CPU.
	 */
	psci_afflvl_suspend(&ep,
			    MPIDR_AFFLVL0,
			    target_afflvl);

	/* Reset PSCI power state parameter for the core. */
	psci_set_suspend_power_state(PSCI_INVALID_DATA);
	return PSCI_E_SUCCESS;
}

int psci_cpu_off(void)
{
	int rc;
	int target_afflvl = PLATFORM_MAX_AFFLVL;

	/*
	 * Traverse from the highest to the lowest affinity level. When the
	 * lowest affinity level is hit, all the locks are acquired. State
	 * management is done immediately followed by cpu, cluster ...
	 * ..target_afflvl specific actions as this function unwinds back.
	 */
	rc = psci_afflvl_off(MPIDR_AFFLVL0, target_afflvl);

	/*
	 * The only error cpu_off can return is E_DENIED. So check if that's
	 * indeed the case.
	 */
	assert (rc == PSCI_E_DENIED);

	return rc;
}

int psci_affinity_info(unsigned long target_affinity,
		       unsigned int lowest_affinity_level)
{
	int rc = PSCI_E_INVALID_PARAMS;
	unsigned int aff_state;
	aff_map_node_t *node;

	if (lowest_affinity_level > PLATFORM_MAX_AFFLVL)
		return rc;

	node = psci_get_aff_map_node(target_affinity, lowest_affinity_level);
	if (node && (node->state & PSCI_AFF_PRESENT)) {

		/*
		 * TODO: For affinity levels higher than 0 i.e. cpu, the
		 * state will always be either ON or OFF. Need to investigate
		 * how critical is it to support ON_PENDING here.
		 */
		aff_state = psci_get_state(node);

		/* A suspended cpu is available & on for the OS */
		if (aff_state == PSCI_STATE_SUSPEND) {
			aff_state = PSCI_STATE_ON;
		}

		rc = aff_state;
	}

	return rc;
}

int psci_migrate(unsigned long target_cpu)
{
	int rc;
	unsigned long resident_cpu_mpidr;

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
	rc = psci_validate_mpidr(target_cpu, MPIDR_AFFLVL0);
	if (rc != PSCI_E_SUCCESS)
		return PSCI_E_INVALID_PARAMS;

	assert(psci_spd_pm && psci_spd_pm->svc_migrate);

	rc = psci_spd_pm->svc_migrate(read_mpidr_el1(), target_cpu);
	assert(rc == PSCI_E_SUCCESS || rc == PSCI_E_INTERN_FAIL);

	return rc;
}

int psci_migrate_info_type(void)
{
	unsigned long resident_cpu_mpidr;

	return psci_spd_migrate_info(&resident_cpu_mpidr);
}

long psci_migrate_info_up_cpu(void)
{
	unsigned long resident_cpu_mpidr;
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
	uint32_t local_caps = psci_caps;

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
		 * The trusted firmware uses the original power state format
		 * and does not support OS Initiated Mode.
		 */
		return (FF_PSTATE_ORIG << FF_PSTATE_SHIFT) |
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

		default:
			break;
		}
	}

	WARN("Unimplemented PSCI Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}
