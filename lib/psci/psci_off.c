/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#include <string.h>
#include "psci_private.h"

/******************************************************************************
 * Construct the psci_power_state to request power OFF at all power levels.
 ******************************************************************************/
static void psci_set_power_off_state(psci_power_state_t *state_info)
{
	int lvl;

	for (lvl = PSCI_CPU_PWR_LVL; lvl <= PLAT_MAX_PWR_LVL; lvl++)
		state_info->pwr_domain_state[lvl] = PLAT_MAX_OFF_STATE;
}

/******************************************************************************
 * Top level handler which is called when a cpu wants to power itself down.
 * It's assumed that along with turning the cpu power domain off, power
 * domains at higher levels will be turned off as far as possible. It finds
 * the highest level where a domain has to be powered off by traversing the
 * node information and then performs generic, architectural, platform setup
 * and state management required to turn OFF that power domain and domains
 * below it. e.g. For a cpu that's to be powered OFF, it could mean programming
 * the power controller whereas for a cluster that's to be powered off, it will
 * call the platform specific code which will disable coherency at the
 * interconnect level if the cpu is the last in the cluster and also the
 * program the power controller.
 ******************************************************************************/
int psci_do_cpu_off(unsigned int end_pwrlvl)
{
	int rc = PSCI_E_SUCCESS, idx = plat_my_core_pos();
	psci_power_state_t state_info;

	/*
	 * This function must only be called on platforms where the
	 * CPU_OFF platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->pwr_domain_off);

	/*
	 * This function acquires the lock corresponding to each power
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(end_pwrlvl,
				      idx);

	/*
	 * Call the cpu off handler registered by the Secure Payload Dispatcher
	 * to let it do any bookkeeping. Assume that the SPD always reports an
	 * E_DENIED error if SP refuse to power down
	 */
	if (psci_spd_pm && psci_spd_pm->svc_off) {
		rc = psci_spd_pm->svc_off(0);
		if (rc)
			goto exit;
	}

	/* Construct the psci_power_state for CPU_OFF */
	psci_set_power_off_state(&state_info);

	/*
	 * This function is passed the requested state info and
	 * it returns the negotiated state info for each power level upto
	 * the end level specified.
	 */
	psci_do_state_coordination(end_pwrlvl, &state_info);

#if ENABLE_PSCI_STAT
	/* Update the last cpu for each level till end_pwrlvl */
	psci_stats_update_pwr_down(end_pwrlvl, &state_info);
#endif

	/*
	 * Arch. management. Perform the necessary steps to flush all
	 * cpu caches.
	 */
	psci_do_pwrdown_cache_maintenance(psci_find_max_off_lvl(&state_info));

	/*
	 * Plat. management: Perform platform specific actions to turn this
	 * cpu off e.g. exit cpu coherency, program the power controller etc.
	 */
	psci_plat_pm_ops->pwr_domain_off(&state_info);

#if ENABLE_PSCI_STAT
	/*
	 * Capture time-stamp while entering low power state.
	 * No cache maintenance needed because caches are off
	 * and writes are direct to main memory.
	 */
	PMF_CAPTURE_TIMESTAMP(psci_svc, PSCI_STAT_ID_ENTER_LOW_PWR,
		PMF_NO_CACHE_MAINT);
#endif

exit:
	/*
	 * Release the locks corresponding to each power level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl,
				      idx);

	/*
	 * Check if all actions needed to safely power down this cpu have
	 * successfully completed.
	 */
	if (rc == PSCI_E_SUCCESS) {
		/*
		 * Set the affinity info state to OFF. This writes directly to
		 * main memory as caches are disabled, so cache maintenance is
		 * required to ensure that later cached reads of aff_info_state
		 * return AFF_STATE_OFF.  A dsbish() ensures ordering of the
		 * update to the affinity info state prior to cache line
		 * invalidation.
		 */
		flush_cpu_data(psci_svc_cpu_data.aff_info_state);
		psci_set_aff_info_state(AFF_STATE_OFF);
		dsbish();
		inv_cpu_data(psci_svc_cpu_data.aff_info_state);

		if (psci_plat_pm_ops->pwr_domain_pwr_down_wfi) {
			/* This function must not return */
			psci_plat_pm_ops->pwr_domain_pwr_down_wfi(&state_info);
		} else {
			/*
			 * Enter a wfi loop which will allow the power
			 * controller to physically power down this cpu.
			 */
			psci_power_down_wfi();
		}
	}

	return rc;
}
