/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <lib/pmf/pmf.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#if ENABLE_PSCI_STAT && ENABLE_PMF
#pragma weak plat_psci_stat_accounting_start
#pragma weak plat_psci_stat_accounting_stop
#pragma weak plat_psci_stat_get_residency

/* Ticks elapsed in one second by a signal of 1 MHz */
#define MHZ_TICKS_PER_SEC 1000000U

/* Maximum time-stamp value read from architectural counters */
#ifdef AARCH32
#define MAX_TS	UINT32_MAX
#else
#define MAX_TS	UINT64_MAX
#endif

/* Following are used as ID's to capture time-stamp */
#define PSCI_STAT_ID_ENTER_LOW_PWR		0
#define PSCI_STAT_ID_EXIT_LOW_PWR		1
#define PSCI_STAT_TOTAL_IDS			2

PMF_REGISTER_SERVICE(psci_svc, PMF_PSCI_STAT_SVC_ID, PSCI_STAT_TOTAL_IDS,
	PMF_STORE_ENABLE)

/*
 * This function calculates the stats residency in microseconds,
 * taking in account the wrap around condition.
 */
static u_register_t calc_stat_residency(unsigned long long pwrupts,
	unsigned long long pwrdnts)
{
	/* The divisor to use to convert raw timestamp into microseconds. */
	u_register_t residency_div;
	u_register_t res;

	/*
	 * Calculate divisor so that it can be directly used to
	 * convert time-stamp into microseconds.
	 */
	residency_div = read_cntfrq_el0() / MHZ_TICKS_PER_SEC;
	assert(residency_div > 0U);

	if (pwrupts < pwrdnts)
		res = MAX_TS - pwrdnts + pwrupts;
	else
		res = pwrupts - pwrdnts;

	return res / residency_div;
}

/*
 * Capture timestamp before entering a low power state.
 * No cache maintenance is required when capturing the timestamp.
 * Cache maintenance may be needed when reading these timestamps.
 */
void plat_psci_stat_accounting_start(
	__unused const psci_power_state_t *state_info)
{
	assert(state_info != NULL);
	PMF_CAPTURE_TIMESTAMP(psci_svc, PSCI_STAT_ID_ENTER_LOW_PWR,
		PMF_NO_CACHE_MAINT);
}

/*
 * Capture timestamp after exiting a low power state.
 * No cache maintenance is required when capturing the timestamp.
 * Cache maintenance may be needed when reading these timestamps.
 */
void plat_psci_stat_accounting_stop(
	__unused const psci_power_state_t *state_info)
{
	assert(state_info != NULL);
	PMF_CAPTURE_TIMESTAMP(psci_svc, PSCI_STAT_ID_EXIT_LOW_PWR,
		PMF_NO_CACHE_MAINT);
}

/*
 * Calculate the residency for the given level and power state
 * information.
 */
u_register_t plat_psci_stat_get_residency(unsigned int lvl,
	const psci_power_state_t *state_info,
	int last_cpu_idx)
{
	plat_local_state_t state;
	unsigned long long pwrup_ts = 0, pwrdn_ts = 0;
	unsigned int pmf_flags;

	assert((lvl >= PSCI_CPU_PWR_LVL) && (lvl <= PLAT_MAX_PWR_LVL));
	assert(state_info != NULL);
	assert(last_cpu_idx <= PLATFORM_CORE_COUNT);

	if (lvl == PSCI_CPU_PWR_LVL)
		assert((unsigned int)last_cpu_idx == plat_my_core_pos());

	/*
	 * If power down is requested, then timestamp capture will
	 * be with caches OFF.  Hence we have to do cache maintenance
	 * when reading the timestamp.
	 */
	state = state_info->pwr_domain_state[PSCI_CPU_PWR_LVL];
	if (is_local_state_off(state) != 0) {
		pmf_flags = PMF_CACHE_MAINT;
	} else {
		assert(is_local_state_retn(state) == 1);
		pmf_flags = PMF_NO_CACHE_MAINT;
	}

	PMF_GET_TIMESTAMP_BY_INDEX(psci_svc,
		PSCI_STAT_ID_ENTER_LOW_PWR,
		last_cpu_idx,
		pmf_flags,
		pwrdn_ts);

	PMF_GET_TIMESTAMP_BY_INDEX(psci_svc,
		PSCI_STAT_ID_EXIT_LOW_PWR,
		plat_my_core_pos(),
		pmf_flags,
		pwrup_ts);

	return calc_stat_residency(pwrup_ts, pwrdn_ts);
}
#endif /* ENABLE_PSCI_STAT && ENABLE_PMF */

/*
 * The PSCI generic code uses this API to let the platform participate in state
 * coordination during a power management operation. It compares the platform
 * specific local power states requested by each cpu for a given power domain
 * and returns the coordinated target power state that the domain should
 * enter. A platform assigns a number to a local power state. This default
 * implementation assumes that the platform assigns these numbers in order of
 * increasing depth of the power state i.e. for two power states X & Y, if X < Y
 * then X represents a shallower power state than Y. As a result, the
 * coordinated target local power state for a power domain will be the minimum
 * of the requested local power states.
 */
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PLAT_MAX_OFF_STATE, temp;
	const plat_local_state_t *st = states;
	unsigned int n = ncpu;

	assert(ncpu > 0U);

	do {
		temp = *st;
		st++;
		if (temp < target)
			target = temp;
		n--;
	} while (n > 0U);

	return target;
}
