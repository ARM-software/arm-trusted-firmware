/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/pmf/pmf.h>
#include <lib/runtime_instr.h>
#include <plat/common/platform.h>

#include "psci_private.h"

/******************************************************************************
 * Construct the psci_power_state to request power OFF at all power levels.
 ******************************************************************************/
static void psci_set_power_off_state(psci_power_state_t *state_info)
{
	unsigned int lvl;

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
	int rc = PSCI_E_SUCCESS;
	unsigned int idx = plat_my_core_pos();
	psci_power_state_t state_info;
	unsigned int parent_nodes[PLAT_MAX_PWR_LVL] = {0};

	/*
	 * This function must only be called on platforms where the
	 * CPU_OFF platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->pwr_domain_off != NULL);

	/* Construct the psci_power_state for CPU_OFF */
	psci_set_power_off_state(&state_info);

	/*
	 * Call the platform provided early CPU_OFF handler to allow
	 * platforms to perform any housekeeping activities before
	 * actually powering the CPU off. PSCI_E_DENIED indicates that
	 * the CPU off sequence should be aborted at this time.
	 */
	if (psci_plat_pm_ops->pwr_domain_off_early) {
		rc = psci_plat_pm_ops->pwr_domain_off_early(&state_info);
		if (rc == PSCI_E_DENIED) {
			return rc;
		}
	}

	/*
	 * Get the parent nodes here, this is important to do before we
	 * initiate the power down sequence as after that point the core may
	 * have exited coherency and its cache may be disabled, any access to
	 * shared memory after that (such as the parent node lookup in
	 * psci_cpu_pd_nodes) can cause coherency issues on some platforms.
	 */
	psci_get_parent_pwr_domain_nodes(idx, end_pwrlvl, parent_nodes);

	/*
	 * This function acquires the lock corresponding to each power
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(end_pwrlvl, parent_nodes);

	/*
	 * Call the cpu off handler registered by the Secure Payload Dispatcher
	 * to let it do any bookkeeping. Assume that the SPD always reports an
	 * E_DENIED error if SP refuse to power down
	 */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_off != NULL)) {
		rc = psci_spd_pm->svc_off(0);
		if (rc != 0)
			goto exit;
	}

	/*
	 * This function is passed the requested state info and
	 * it returns the negotiated state info for each power level upto
	 * the end level specified.
	 */
	psci_do_state_coordination(idx, end_pwrlvl, &state_info);

	/* Update the target state in the power domain nodes */
	psci_set_target_local_pwr_states(idx, end_pwrlvl, &state_info);

#if ENABLE_PSCI_STAT
	/* Update the last cpu for each level till end_pwrlvl */
	psci_stats_update_pwr_down(idx, end_pwrlvl, &state_info);
#endif

	/*
	 * Arch. management. Initiate power down sequence.
	 */
	psci_pwrdown_cpu(psci_find_max_off_lvl(&state_info));

	/*
	 * Plat. management: Perform platform specific actions to turn this
	 * cpu off e.g. exit cpu coherency, program the power controller etc.
	 */
	psci_plat_pm_ops->pwr_domain_off(&state_info);

#if ENABLE_PSCI_STAT
	plat_psci_stat_accounting_start(&state_info);
#endif

exit:
	/*
	 * Release the locks corresponding to each power level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl, parent_nodes);

	/*
	 * Check if all actions needed to safely power down this cpu have
	 * successfully completed.
	 */
	if (rc == PSCI_E_SUCCESS) {
		/*
		 * Set the affinity info state to OFF. When caches are disabled,
		 * this writes directly to main memory, so cache maintenance is
		 * required to ensure that later cached reads of aff_info_state
		 * return AFF_STATE_OFF. A dsbish() ensures ordering of the
		 * update to the affinity info state prior to cache line
		 * invalidation.
		 */
		psci_flush_cpu_data(psci_svc_cpu_data.aff_info_state);
		psci_set_aff_info_state(AFF_STATE_OFF);
		psci_dsbish();
		psci_inv_cpu_data(psci_svc_cpu_data.aff_info_state);

#if ENABLE_RUNTIME_INSTRUMENTATION

		/*
		 * Update the timestamp with cache off.  We assume this
		 * timestamp can only be read from the current CPU and the
		 * timestamp cache line will be flushed before return to
		 * normal world on wakeup.
		 */
		PMF_CAPTURE_TIMESTAMP(rt_instr_svc,
		    RT_INSTR_ENTER_HW_LOW_PWR,
		    PMF_NO_CACHE_MAINT);
#endif

		if (psci_plat_pm_ops->pwr_domain_pwr_down_wfi != NULL) {
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
