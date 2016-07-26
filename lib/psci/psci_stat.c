/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include "psci_private.h"

#ifndef PLAT_MAX_PWR_LVL_STATES
#define PLAT_MAX_PWR_LVL_STATES 2
#endif

/* Ticks elapsed in one second by a signal of 1 MHz */
#define MHZ_TICKS_PER_SEC 1000000

/* Following structure is used for PSCI STAT */
typedef struct psci_stat {
	u_register_t residency;
	u_register_t count;
} psci_stat_t;

/*
 * Following is used to keep track of the last cpu
 * that goes to power down in non cpu power domains.
 */
static int last_cpu_in_non_cpu_pd[PSCI_NUM_NON_CPU_PWR_DOMAINS] = {-1};

/*
 * Following are used to store PSCI STAT values for
 * CPU and non CPU power domains.
 */
static psci_stat_t psci_cpu_stat[PLATFORM_CORE_COUNT]
				[PLAT_MAX_PWR_LVL_STATES];
static psci_stat_t psci_non_cpu_stat[PSCI_NUM_NON_CPU_PWR_DOMAINS]
				[PLAT_MAX_PWR_LVL_STATES];

/* Register PMF PSCI service */
PMF_REGISTER_SERVICE(psci_svc, PMF_PSCI_STAT_SVC_ID,
	 PSCI_STAT_TOTAL_IDS, PMF_STORE_ENABLE)

/* The divisor to use to convert raw timestamp into microseconds */
u_register_t residency_div;

/*
 * This macro calculates the stats residency in microseconds,
 * taking in account the wrap around condition.
 */
#define calc_stat_residency(_pwrupts, _pwrdnts, _res)		\
	do {							\
		if (_pwrupts < _pwrdnts)			\
			_res = UINT64_MAX - _pwrdnts + _pwrupts;\
		else						\
			_res = _pwrupts - _pwrdnts;		\
		/* Convert timestamp into microseconds */	\
		_res = _res/residency_div;			\
	} while (0)

/*
 * This functions returns the index into the `psci_stat_t` array given the
 * local power state and power domain level. If the platform implements the
 * `get_pwr_lvl_state_idx` pm hook, then that will be used to return the index.
 */
static int get_stat_idx(plat_local_state_t local_state, int pwr_lvl)
{
	int idx;

	if (psci_plat_pm_ops->get_pwr_lvl_state_idx == NULL) {
		assert(PLAT_MAX_PWR_LVL_STATES == 2);
		if (is_local_state_retn(local_state))
			return 0;

		assert(is_local_state_off(local_state));
		return 1;
	}

	idx = psci_plat_pm_ops->get_pwr_lvl_state_idx(local_state, pwr_lvl);
	assert((idx >= 0) && (idx < PLAT_MAX_PWR_LVL_STATES));
	return idx;
}

/*******************************************************************************
 * This function is passed the target local power states for each power
 * domain (state_info) between the current CPU domain and its ancestors until
 * the target power level (end_pwrlvl).
 *
 * Then, for each level (apart from the CPU level) until the 'end_pwrlvl', it
 * updates the `last_cpu_in_non_cpu_pd[]` with last power down cpu id.
 *
 * This function will only be invoked with data cache enabled and while
 * powering down a core.
 ******************************************************************************/
void psci_stats_update_pwr_down(unsigned int end_pwrlvl,
			const psci_power_state_t *state_info)
{
	int lvl, parent_idx, cpu_idx = plat_my_core_pos();

	assert(end_pwrlvl <= PLAT_MAX_PWR_LVL);
	assert(state_info);

	parent_idx = psci_cpu_pd_nodes[cpu_idx].parent_node;

	for (lvl = PSCI_CPU_PWR_LVL + 1; lvl <= end_pwrlvl; lvl++) {

		/* Break early if the target power state is RUN */
		if (is_local_state_run(state_info->pwr_domain_state[lvl]))
			break;

		/*
		 * The power domain is entering a low power state, so this is
		 * the last CPU for this power domain
		 */
		last_cpu_in_non_cpu_pd[parent_idx] = cpu_idx;

		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

}

/*******************************************************************************
 * This function updates the PSCI STATS(residency time and count) for CPU
 * and NON-CPU power domains.
 * It is called with caches enabled and locks acquired(for NON-CPU domain)
 ******************************************************************************/
void psci_stats_update_pwr_up(unsigned int end_pwrlvl,
			const psci_power_state_t *state_info,
			unsigned int flags)
{
	int parent_idx, cpu_idx = plat_my_core_pos();
	int lvl, stat_idx;
	plat_local_state_t local_state;
	unsigned long long pwrup_ts = 0, pwrdn_ts = 0;
	u_register_t residency;

	assert(end_pwrlvl <= PLAT_MAX_PWR_LVL);
	assert(state_info);

	/* Initialize the residency divisor if not already initialized */
	if (!residency_div) {
		/* Pre-calculate divisor so that it can be directly used to
		   convert time-stamp into microseconds */
		residency_div = read_cntfrq_el0() / MHZ_TICKS_PER_SEC;
		assert(residency_div);
	}

	/* Get power down time-stamp for current CPU */
	PMF_GET_TIMESTAMP_BY_INDEX(psci_svc, PSCI_STAT_ID_ENTER_LOW_PWR,
			cpu_idx, flags, pwrdn_ts);

	/* In the case of 1st power on just return */
	if (!pwrdn_ts)
		return;

	/* Get power up time-stamp for current CPU */
	PMF_GET_TIMESTAMP_BY_INDEX(psci_svc, PSCI_STAT_ID_EXIT_LOW_PWR,
			cpu_idx, flags, pwrup_ts);

	/* Get the index into the stats array */
	local_state = state_info->pwr_domain_state[PSCI_CPU_PWR_LVL];
	stat_idx = get_stat_idx(local_state, PSCI_CPU_PWR_LVL);

	/* Calculate stats residency */
	calc_stat_residency(pwrup_ts, pwrdn_ts, residency);

	/* Update CPU stats. */
	psci_cpu_stat[cpu_idx][stat_idx].residency += residency;
	psci_cpu_stat[cpu_idx][stat_idx].count++;

	/*
	 * Check what power domains above CPU were off
	 * prior to this CPU powering on.
	 */
	parent_idx = psci_cpu_pd_nodes[cpu_idx].parent_node;
	for (lvl = PSCI_CPU_PWR_LVL + 1; lvl <= end_pwrlvl; lvl++) {
		local_state = state_info->pwr_domain_state[lvl];
		if (is_local_state_run(local_state)) {
			/* Break early */
			break;
		}

		assert(last_cpu_in_non_cpu_pd[parent_idx] != -1);

		/* Get power down time-stamp for last CPU */
		PMF_GET_TIMESTAMP_BY_INDEX(psci_svc, PSCI_STAT_ID_ENTER_LOW_PWR,
				last_cpu_in_non_cpu_pd[parent_idx],
				flags, pwrdn_ts);

		/* Initialize back to reset value */
		last_cpu_in_non_cpu_pd[parent_idx] = -1;

		/* Get the index into the stats array */
		stat_idx = get_stat_idx(local_state, lvl);

		/* Calculate stats residency */
		calc_stat_residency(pwrup_ts, pwrdn_ts, residency);

		/* Update non cpu stats */
		psci_non_cpu_stat[parent_idx][stat_idx].residency += residency;
		psci_non_cpu_stat[parent_idx][stat_idx].count++;

		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

}

/*******************************************************************************
 * This function returns the appropriate count and residency time of the
 * local state for the highest power level expressed in the `power_state`
 * for the node represented by `target_cpu`.
 ******************************************************************************/
int psci_get_stat(u_register_t target_cpu, unsigned int power_state,
			 psci_stat_t *psci_stat)
{
	int rc, pwrlvl, lvl, parent_idx, stat_idx, target_idx;
	psci_power_state_t state_info = { {PSCI_LOCAL_STATE_RUN} };
	plat_local_state_t local_state;

	/* Validate the target_cpu parameter and determine the cpu index */
	target_idx = plat_core_pos_by_mpidr(target_cpu);
	if (target_idx == -1)
		return PSCI_E_INVALID_PARAMS;

	/* Validate the power_state parameter */
	if (!psci_plat_pm_ops->translate_power_state_by_mpidr)
		rc = psci_validate_power_state(power_state, &state_info);
	else
		rc = psci_plat_pm_ops->translate_power_state_by_mpidr(
				target_cpu, power_state, &state_info);

	if (rc != PSCI_E_SUCCESS)
		return PSCI_E_INVALID_PARAMS;

	/* Find the highest power level */
	pwrlvl = psci_find_target_suspend_lvl(&state_info);
	if (pwrlvl == PSCI_INVALID_PWR_LVL) {
		ERROR("Invalid target power level for PSCI statistics operation\n");
		panic();
	}

	/* Get the index into the stats array */
	local_state = state_info.pwr_domain_state[pwrlvl];
	stat_idx = get_stat_idx(local_state, pwrlvl);

	if (pwrlvl > PSCI_CPU_PWR_LVL) {
		/* Get the power domain index */
		parent_idx = psci_cpu_pd_nodes[target_idx].parent_node;
		for (lvl = PSCI_CPU_PWR_LVL + 1; lvl < pwrlvl; lvl++)
			parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;

		/* Get the non cpu power domain stats */
		*psci_stat = psci_non_cpu_stat[parent_idx][stat_idx];
	} else {
		/* Get the cpu power domain stats */
		*psci_stat = psci_cpu_stat[target_idx][stat_idx];
	}

	return PSCI_E_SUCCESS;
}

/* This is the top level function for PSCI_STAT_RESIDENCY SMC. */
u_register_t psci_stat_residency(u_register_t target_cpu,
		unsigned int power_state)
{
	psci_stat_t psci_stat;

	int rc = psci_get_stat(target_cpu, power_state, &psci_stat);
	if (rc == PSCI_E_SUCCESS)
		return psci_stat.residency;
	else
		return 0;
}

/* This is the top level function for PSCI_STAT_COUNT SMC. */
u_register_t psci_stat_count(u_register_t target_cpu,
	unsigned int power_state)
{
	psci_stat_t psci_stat;

	int rc = psci_get_stat(target_cpu, power_state, &psci_stat);
	if (rc == PSCI_E_SUCCESS)
		return psci_stat.count;
	else
		return 0;
}
