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
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform.h>
#include <string.h>
#include "psci_private.h"

/*
 * SPD power management operations, expected to be supplied by the registered
 * SPD on successful SP initialization
 */
const spd_pm_ops_t *psci_spd_pm;

/*
 * PSCI requested local power state map. This array is used to store the local
 * power states requested by a CPU for power levels from level 1 to
 * PLAT_MAX_PWR_LVL. It does not store the requested local power state for power
 * level 0 (PSCI_CPU_PWR_LVL) as the requested and the target power state for a
 * CPU are the same.
 *
 * During state coordination, the platform is passed an array containing the
 * local states requested for a particular non cpu power domain by each cpu
 * within the domain.
 *
 * TODO: Dense packing of the requested states will cause cache thrashing
 * when multiple power domains write to it. If we allocate the requested
 * states at each power level in a cache-line aligned per-domain memory,
 * the cache thrashing can be avoided.
 */
static plat_local_state_t
	psci_req_local_pwr_states[PLAT_MAX_PWR_LVL][PLATFORM_CORE_COUNT];


/*******************************************************************************
 * Arrays that hold the platform's power domain tree information for state
 * management of power domains.
 * Each node in the array 'psci_non_cpu_pd_nodes' corresponds to a power domain
 * which is an ancestor of a CPU power domain.
 * Each node in the array 'psci_cpu_pd_nodes' corresponds to a cpu power domain
 ******************************************************************************/
non_cpu_pd_node_t psci_non_cpu_pd_nodes[PSCI_NUM_NON_CPU_PWR_DOMAINS]
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;

DEFINE_BAKERY_LOCK(psci_locks[PSCI_NUM_NON_CPU_PWR_DOMAINS]);

cpu_pd_node_t psci_cpu_pd_nodes[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Pointer to functions exported by the platform to complete power mgmt. ops
 ******************************************************************************/
const plat_psci_ops_t *psci_plat_pm_ops;

/******************************************************************************
 * Check that the maximum power level supported by the platform makes sense
 *****************************************************************************/
CASSERT(PLAT_MAX_PWR_LVL <= PSCI_MAX_PWR_LVL && \
		PLAT_MAX_PWR_LVL >= PSCI_CPU_PWR_LVL, \
		assert_platform_max_pwrlvl_check);

/*
 * The plat_local_state used by the platform is one of these types: RUN,
 * RETENTION and OFF. The platform can define further sub-states for each type
 * apart from RUN. This categorization is done to verify the sanity of the
 * psci_power_state passed by the platform and to print debug information. The
 * categorization is done on the basis of the following conditions:
 *
 * 1. If (plat_local_state == 0) then the category is STATE_TYPE_RUN.
 *
 * 2. If (0 < plat_local_state <= PLAT_MAX_RET_STATE), then the category is
 *    STATE_TYPE_RETN.
 *
 * 3. If (plat_local_state > PLAT_MAX_RET_STATE), then the category is
 *    STATE_TYPE_OFF.
 */
typedef enum plat_local_state_type {
	STATE_TYPE_RUN = 0,
	STATE_TYPE_RETN,
	STATE_TYPE_OFF
} plat_local_state_type_t;

/* The macro used to categorize plat_local_state. */
#define find_local_state_type(plat_local_state)					\
		((plat_local_state) ? ((plat_local_state > PLAT_MAX_RET_STATE)	\
		? STATE_TYPE_OFF : STATE_TYPE_RETN)				\
		: STATE_TYPE_RUN)

/******************************************************************************
 * Check that the maximum retention level supported by the platform is less
 * than the maximum off level.
 *****************************************************************************/
CASSERT(PLAT_MAX_RET_STATE < PLAT_MAX_OFF_STATE, \
		assert_platform_max_off_and_retn_state_check);

/******************************************************************************
 * This function ensures that the power state parameter in a CPU_SUSPEND request
 * is valid. If so, it returns the requested states for each power level.
 *****************************************************************************/
int psci_validate_power_state(unsigned int power_state,
			      psci_power_state_t *state_info)
{
	/* Check SBZ bits in power state are zero */
	if (psci_check_power_state(power_state))
		return PSCI_E_INVALID_PARAMS;

	assert(psci_plat_pm_ops->validate_power_state);

	/* Validate the power_state using platform pm_ops */
	return psci_plat_pm_ops->validate_power_state(power_state, state_info);
}

/******************************************************************************
 * This function retrieves the `psci_power_state_t` for system suspend from
 * the platform.
 *****************************************************************************/
void psci_query_sys_suspend_pwrstate(psci_power_state_t *state_info)
{
	/*
	 * Assert that the required pm_ops hook is implemented to ensure that
	 * the capability detected during psci_setup() is valid.
	 */
	assert(psci_plat_pm_ops->get_sys_suspend_power_state);

	/*
	 * Query the platform for the power_state required for system suspend
	 */
	psci_plat_pm_ops->get_sys_suspend_power_state(state_info);
}

/*******************************************************************************
 * This function verifies that the all the other cores in the system have been
 * turned OFF and the current CPU is the last running CPU in the system.
 * Returns 1 (true) if the current CPU is the last ON CPU or 0 (false)
 * otherwise.
 ******************************************************************************/
unsigned int psci_is_last_on_cpu(void)
{
	unsigned int cpu_idx, my_idx = plat_my_core_pos();

	for (cpu_idx = 0; cpu_idx < PLATFORM_CORE_COUNT; cpu_idx++) {
		if (cpu_idx == my_idx) {
			assert(psci_get_aff_info_state() == AFF_STATE_ON);
			continue;
		}

		if (psci_get_aff_info_state_by_idx(cpu_idx) != AFF_STATE_OFF)
			return 0;
	}

	return 1;
}

/*******************************************************************************
 * Routine to return the maximum power level to traverse to after a cpu has
 * been physically powered up. It is expected to be called immediately after
 * reset from assembler code.
 ******************************************************************************/
static unsigned int get_power_on_target_pwrlvl(void)
{
	unsigned int pwrlvl;

	/*
	 * Assume that this cpu was suspended and retrieve its target power
	 * level. If it is invalid then it could only have been turned off
	 * earlier. PLAT_MAX_PWR_LVL will be the highest power level a
	 * cpu can be turned off to.
	 */
	pwrlvl = psci_get_suspend_pwrlvl();
	if (pwrlvl == PSCI_INVALID_PWR_LVL)
		pwrlvl = PLAT_MAX_PWR_LVL;
	return pwrlvl;
}

/******************************************************************************
 * Helper function to update the requested local power state array. This array
 * does not store the requested state for the CPU power level. Hence an
 * assertion is added to prevent us from accessing the wrong index.
 *****************************************************************************/
static void psci_set_req_local_pwr_state(unsigned int pwrlvl,
					 unsigned int cpu_idx,
					 plat_local_state_t req_pwr_state)
{
	assert(pwrlvl > PSCI_CPU_PWR_LVL);
	psci_req_local_pwr_states[pwrlvl - 1][cpu_idx] = req_pwr_state;
}

/******************************************************************************
 * This function initializes the psci_req_local_pwr_states.
 *****************************************************************************/
void psci_init_req_local_pwr_states(void)
{
	/* Initialize the requested state of all non CPU power domains as OFF */
	memset(&psci_req_local_pwr_states, PLAT_MAX_OFF_STATE,
			sizeof(psci_req_local_pwr_states));
}

/******************************************************************************
 * Helper function to return a reference to an array containing the local power
 * states requested by each cpu for a power domain at 'pwrlvl'. The size of the
 * array will be the number of cpu power domains of which this power domain is
 * an ancestor. These requested states will be used to determine a suitable
 * target state for this power domain during psci state coordination. An
 * assertion is added to prevent us from accessing the CPU power level.
 *****************************************************************************/
static plat_local_state_t *psci_get_req_local_pwr_states(unsigned int pwrlvl,
							 unsigned int cpu_idx)
{
	assert(pwrlvl > PSCI_CPU_PWR_LVL);

	return &psci_req_local_pwr_states[pwrlvl - 1][cpu_idx];
}

/******************************************************************************
 * Helper function to return the current local power state of each power domain
 * from the current cpu power domain to its ancestor at the 'end_pwrlvl'. This
 * function will be called after a cpu is powered on to find the local state
 * each power domain has emerged from.
 *****************************************************************************/
static void psci_get_target_local_pwr_states(unsigned int end_pwrlvl,
					     psci_power_state_t *target_state)
{
	unsigned int parent_idx, lvl;
	plat_local_state_t *pd_state = target_state->pwr_domain_state;

	pd_state[PSCI_CPU_PWR_LVL] = psci_get_cpu_local_state();
	parent_idx = psci_cpu_pd_nodes[plat_my_core_pos()].parent_node;

	/* Copy the local power state from node to state_info */
	for (lvl = PSCI_CPU_PWR_LVL + 1; lvl <= end_pwrlvl; lvl++) {
#if !USE_COHERENT_MEM
		/*
		 * If using normal memory for psci_non_cpu_pd_nodes, we need
		 * to flush before reading the local power state as another
		 * cpu in the same power domain could have updated it and this
		 * code runs before caches are enabled.
		 */
		flush_dcache_range(
				(uintptr_t) &psci_non_cpu_pd_nodes[parent_idx],
				sizeof(psci_non_cpu_pd_nodes[parent_idx]));
#endif
		pd_state[lvl] =	psci_non_cpu_pd_nodes[parent_idx].local_state;
		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

	/* Set the the higher levels to RUN */
	for (; lvl <= PLAT_MAX_PWR_LVL; lvl++)
		target_state->pwr_domain_state[lvl] = PSCI_LOCAL_STATE_RUN;
}

/******************************************************************************
 * Helper function to set the target local power state that each power domain
 * from the current cpu power domain to its ancestor at the 'end_pwrlvl' will
 * enter. This function will be called after coordination of requested power
 * states has been done for each power level.
 *****************************************************************************/
static void psci_set_target_local_pwr_states(unsigned int end_pwrlvl,
					const psci_power_state_t *target_state)
{
	unsigned int parent_idx, lvl;
	const plat_local_state_t *pd_state = target_state->pwr_domain_state;

	psci_set_cpu_local_state(pd_state[PSCI_CPU_PWR_LVL]);

	/*
	 * Need to flush as local_state will be accessed with Data Cache
	 * disabled during power on
	 */
	flush_cpu_data(psci_svc_cpu_data.local_state);

	parent_idx = psci_cpu_pd_nodes[plat_my_core_pos()].parent_node;

	/* Copy the local_state from state_info */
	for (lvl = 1; lvl <= end_pwrlvl; lvl++) {
		psci_non_cpu_pd_nodes[parent_idx].local_state =	pd_state[lvl];
#if !USE_COHERENT_MEM
		flush_dcache_range(
				(uintptr_t)&psci_non_cpu_pd_nodes[parent_idx],
				sizeof(psci_non_cpu_pd_nodes[parent_idx]));
#endif
		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}
}


/*******************************************************************************
 * PSCI helper function to get the parent nodes corresponding to a cpu_index.
 ******************************************************************************/
void psci_get_parent_pwr_domain_nodes(unsigned int cpu_idx,
				      unsigned int end_lvl,
				      unsigned int node_index[])
{
	unsigned int parent_node = psci_cpu_pd_nodes[cpu_idx].parent_node;
	int i;

	for (i = PSCI_CPU_PWR_LVL + 1; i <= end_lvl; i++) {
		*node_index++ = parent_node;
		parent_node = psci_non_cpu_pd_nodes[parent_node].parent_node;
	}
}

/******************************************************************************
 * This function is invoked post CPU power up and initialization. It sets the
 * affinity info state, target power state and requested power state for the
 * current CPU and all its ancestor power domains to RUN.
 *****************************************************************************/
void psci_set_pwr_domains_to_run(unsigned int end_pwrlvl)
{
	unsigned int parent_idx, cpu_idx = plat_my_core_pos(), lvl;
	parent_idx = psci_cpu_pd_nodes[cpu_idx].parent_node;

	/* Reset the local_state to RUN for the non cpu power domains. */
	for (lvl = PSCI_CPU_PWR_LVL + 1; lvl <= end_pwrlvl; lvl++) {
		psci_non_cpu_pd_nodes[parent_idx].local_state =
				PSCI_LOCAL_STATE_RUN;
#if !USE_COHERENT_MEM
		flush_dcache_range(
				(uintptr_t) &psci_non_cpu_pd_nodes[parent_idx],
				sizeof(psci_non_cpu_pd_nodes[parent_idx]));
#endif
		psci_set_req_local_pwr_state(lvl,
					     cpu_idx,
					     PSCI_LOCAL_STATE_RUN);
		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

	/* Set the affinity info state to ON */
	psci_set_aff_info_state(AFF_STATE_ON);

	psci_set_cpu_local_state(PSCI_LOCAL_STATE_RUN);
	flush_cpu_data(psci_svc_cpu_data);
}

/******************************************************************************
 * This function is passed the local power states requested for each power
 * domain (state_info) between the current CPU domain and its ancestors until
 * the target power level (end_pwrlvl). It updates the array of requested power
 * states with this information.
 *
 * Then, for each level (apart from the CPU level) until the 'end_pwrlvl', it
 * retrieves the states requested by all the cpus of which the power domain at
 * that level is an ancestor. It passes this information to the platform to
 * coordinate and return the target power state. If the target state for a level
 * is RUN then subsequent levels are not considered. At the CPU level, state
 * coordination is not required. Hence, the requested and the target states are
 * the same.
 *
 * The 'state_info' is updated with the target state for each level between the
 * CPU and the 'end_pwrlvl' and returned to the caller.
 *
 * This function will only be invoked with data cache enabled and while
 * powering down a core.
 *****************************************************************************/
void psci_do_state_coordination(unsigned int end_pwrlvl,
				psci_power_state_t *state_info)
{
	unsigned int lvl, parent_idx, cpu_idx = plat_my_core_pos();
	unsigned int start_idx, ncpus;
	plat_local_state_t target_state, *req_states;

	parent_idx = psci_cpu_pd_nodes[cpu_idx].parent_node;

	/* For level 0, the requested state will be equivalent
	   to target state */
	for (lvl = PSCI_CPU_PWR_LVL + 1; lvl <= end_pwrlvl; lvl++) {

		/* First update the requested power state */
		psci_set_req_local_pwr_state(lvl, cpu_idx,
					     state_info->pwr_domain_state[lvl]);

		/* Get the requested power states for this power level */
		start_idx = psci_non_cpu_pd_nodes[parent_idx].cpu_start_idx;
		req_states = psci_get_req_local_pwr_states(lvl, start_idx);

		/*
		 * Let the platform coordinate amongst the requested states at
		 * this power level and return the target local power state.
		 */
		ncpus = psci_non_cpu_pd_nodes[parent_idx].ncpus;
		target_state = plat_get_target_pwr_state(lvl,
							 req_states,
							 ncpus);

		state_info->pwr_domain_state[lvl] = target_state;

		/* Break early if the negotiated target power state is RUN */
		if (is_local_state_run(state_info->pwr_domain_state[lvl]))
			break;

		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

	/*
	 * This is for cases when we break out of the above loop early because
	 * the target power state is RUN at a power level < end_pwlvl.
	 * We update the requested power state from state_info and then
	 * set the target state as RUN.
	 */
	for (lvl = lvl + 1; lvl <= end_pwrlvl; lvl++) {
		psci_set_req_local_pwr_state(lvl, cpu_idx,
					     state_info->pwr_domain_state[lvl]);
		state_info->pwr_domain_state[lvl] = PSCI_LOCAL_STATE_RUN;

	}

	/* Update the target state in the power domain nodes */
	psci_set_target_local_pwr_states(end_pwrlvl, state_info);
}

/******************************************************************************
 * This function validates a suspend request by making sure that if a standby
 * state is requested then no power level is turned off and the highest power
 * level is placed in a standby/retention state.
 *
 * It also ensures that the state level X will enter is not shallower than the
 * state level X + 1 will enter.
 *
 * This validation will be enabled only for DEBUG builds as the platform is
 * expected to perform these validations as well.
 *****************************************************************************/
int psci_validate_suspend_req(const psci_power_state_t *state_info,
			      unsigned int is_power_down_state)
{
	unsigned int max_off_lvl, target_lvl, max_retn_lvl;
	plat_local_state_t state;
	plat_local_state_type_t req_state_type, deepest_state_type;
	int i;

	/* Find the target suspend power level */
	target_lvl = psci_find_target_suspend_lvl(state_info);
	if (target_lvl == PSCI_INVALID_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* All power domain levels are in a RUN state to begin with */
	deepest_state_type = STATE_TYPE_RUN;

	for (i = target_lvl; i >= PSCI_CPU_PWR_LVL; i--) {
		state = state_info->pwr_domain_state[i];
		req_state_type = find_local_state_type(state);

		/*
		 * While traversing from the highest power level to the lowest,
		 * the state requested for lower levels has to be the same or
		 * deeper i.e. equal to or greater than the state at the higher
		 * levels. If this condition is true, then the requested state
		 * becomes the deepest state encountered so far.
		 */
		if (req_state_type < deepest_state_type)
			return PSCI_E_INVALID_PARAMS;
		deepest_state_type = req_state_type;
	}

	/* Find the highest off power level */
	max_off_lvl = psci_find_max_off_lvl(state_info);

	/* The target_lvl is either equal to the max_off_lvl or max_retn_lvl */
	max_retn_lvl = PSCI_INVALID_PWR_LVL;
	if (target_lvl != max_off_lvl)
		max_retn_lvl = target_lvl;

	/*
	 * If this is not a request for a power down state then max off level
	 * has to be invalid and max retention level has to be a valid power
	 * level.
	 */
	if (!is_power_down_state && (max_off_lvl != PSCI_INVALID_PWR_LVL ||
				    max_retn_lvl == PSCI_INVALID_PWR_LVL))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/******************************************************************************
 * This function finds the highest power level which will be powered down
 * amongst all the power levels specified in the 'state_info' structure
 *****************************************************************************/
unsigned int psci_find_max_off_lvl(const psci_power_state_t *state_info)
{
	int i;

	for (i = PLAT_MAX_PWR_LVL; i >= PSCI_CPU_PWR_LVL; i--) {
		if (is_local_state_off(state_info->pwr_domain_state[i]))
			return i;
	}

	return PSCI_INVALID_PWR_LVL;
}

/******************************************************************************
 * This functions finds the level of the highest power domain which will be
 * placed in a low power state during a suspend operation.
 *****************************************************************************/
unsigned int psci_find_target_suspend_lvl(const psci_power_state_t *state_info)
{
	int i;

	for (i = PLAT_MAX_PWR_LVL; i >= PSCI_CPU_PWR_LVL; i--) {
		if (!is_local_state_run(state_info->pwr_domain_state[i]))
			return i;
	}

	return PSCI_INVALID_PWR_LVL;
}

/*******************************************************************************
 * This function is passed a cpu_index and the highest level in the topology
 * tree that the operation should be applied to. It picks up locks in order of
 * increasing power domain level in the range specified.
 ******************************************************************************/
void psci_acquire_pwr_domain_locks(unsigned int end_pwrlvl,
				   unsigned int cpu_idx)
{
	unsigned int parent_idx = psci_cpu_pd_nodes[cpu_idx].parent_node;
	unsigned int level;

	/* No locking required for level 0. Hence start locking from level 1 */
	for (level = PSCI_CPU_PWR_LVL + 1; level <= end_pwrlvl; level++) {
		psci_lock_get(&psci_non_cpu_pd_nodes[parent_idx]);
		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}
}

/*******************************************************************************
 * This function is passed a cpu_index and the highest level in the topology
 * tree that the operation should be applied to. It releases the locks in order
 * of decreasing power domain level in the range specified.
 ******************************************************************************/
void psci_release_pwr_domain_locks(unsigned int end_pwrlvl,
				   unsigned int cpu_idx)
{
	unsigned int parent_idx, parent_nodes[PLAT_MAX_PWR_LVL] = {0};
	int level;

	/* Get the parent nodes */
	psci_get_parent_pwr_domain_nodes(cpu_idx, end_pwrlvl, parent_nodes);

	/* Unlock top down. No unlocking required for level 0. */
	for (level = end_pwrlvl; level >= PSCI_CPU_PWR_LVL + 1; level--) {
		parent_idx = parent_nodes[level - 1];
		psci_lock_release(&psci_non_cpu_pd_nodes[parent_idx]);
	}
}

/*******************************************************************************
 * Simple routine to determine whether a mpidr is valid or not.
 ******************************************************************************/
int psci_validate_mpidr(u_register_t mpidr)
{
	if (plat_core_pos_by_mpidr(mpidr) < 0)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function determines the full entrypoint information for the requested
 * PSCI entrypoint on power on/resume and returns it.
 ******************************************************************************/
static int psci_get_ns_ep_info(entry_point_info_t *ep,
			       uintptr_t entrypoint,
			       u_register_t context_id)
{
	unsigned long ep_attr, sctlr;
	unsigned int daif, ee, mode;
	unsigned long ns_scr_el3 = read_scr_el3();
	unsigned long ns_sctlr_el1 = read_sctlr_el1();

	sctlr = ns_scr_el3 & SCR_HCE_BIT ? read_sctlr_el2() : ns_sctlr_el1;
	ee = 0;

	ep_attr = NON_SECURE | EP_ST_DISABLE;
	if (sctlr & SCTLR_EE_BIT) {
		ep_attr |= EP_EE_BIG;
		ee = 1;
	}
	SET_PARAM_HEAD(ep, PARAM_EP, VERSION_1, ep_attr);

	ep->pc = entrypoint;
	memset(&ep->args, 0, sizeof(ep->args));
	ep->args.arg0 = context_id;

	/*
	 * Figure out whether the cpu enters the non-secure address space
	 * in aarch32 or aarch64
	 */
	if (ns_scr_el3 & SCR_RW_BIT) {

		/*
		 * Check whether a Thumb entry point has been provided for an
		 * aarch64 EL
		 */
		if (entrypoint & 0x1)
			return PSCI_E_INVALID_ADDRESS;

		mode = ns_scr_el3 & SCR_HCE_BIT ? MODE_EL2 : MODE_EL1;

		ep->spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	} else {

		mode = ns_scr_el3 & SCR_HCE_BIT ? MODE32_hyp : MODE32_svc;

		/*
		 * TODO: Choose async. exception bits if HYP mode is not
		 * implemented according to the values of SCR.{AW, FW} bits
		 */
		daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

		ep->spsr = SPSR_MODE32(mode, entrypoint & 0x1, ee, daif);
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function validates the entrypoint with the platform layer if the
 * appropriate pm_ops hook is exported by the platform and returns the
 * 'entry_point_info'.
 ******************************************************************************/
int psci_validate_entry_point(entry_point_info_t *ep,
			      uintptr_t entrypoint,
			      u_register_t context_id)
{
	int rc;

	/* Validate the entrypoint using platform psci_ops */
	if (psci_plat_pm_ops->validate_ns_entrypoint) {
		rc = psci_plat_pm_ops->validate_ns_entrypoint(entrypoint);
		if (rc != PSCI_E_SUCCESS)
			return PSCI_E_INVALID_ADDRESS;
	}

	/*
	 * Verify and derive the re-entry information for
	 * the non-secure world from the non-secure state from
	 * where this call originated.
	 */
	rc = psci_get_ns_ep_info(ep, entrypoint, context_id);
	return rc;
}

/*******************************************************************************
 * Generic handler which is called when a cpu is physically powered on. It
 * traverses the node information and finds the highest power level powered
 * off and performs generic, architectural, platform setup and state management
 * to power on that power level and power levels below it.
 * e.g. For a cpu that's been powered on, it will call the platform specific
 * code to enable the gic cpu interface and for a cluster it will enable
 * coherency at the interconnect level in addition to gic cpu interface.
 ******************************************************************************/
void psci_power_up_finish(void)
{
	unsigned int end_pwrlvl, cpu_idx = plat_my_core_pos();
	psci_power_state_t state_info = { {PSCI_LOCAL_STATE_RUN} };

	/*
	 * Verify that we have been explicitly turned ON or resumed from
	 * suspend.
	 */
	if (psci_get_aff_info_state() == AFF_STATE_OFF) {
		ERROR("Unexpected affinity info state");
		panic();
	}

	/*
	 * Get the maximum power domain level to traverse to after this cpu
	 * has been physically powered up.
	 */
	end_pwrlvl = get_power_on_target_pwrlvl();

	/*
	 * This function acquires the lock corresponding to each power level so
	 * that by the time all locks are taken, the system topology is snapshot
	 * and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(end_pwrlvl,
				      cpu_idx);

	psci_get_target_local_pwr_states(end_pwrlvl, &state_info);

	/*
	 * This CPU could be resuming from suspend or it could have just been
	 * turned on. To distinguish between these 2 cases, we examine the
	 * affinity state of the CPU:
	 *  - If the affinity state is ON_PENDING then it has just been
	 *    turned on.
	 *  - Else it is resuming from suspend.
	 *
	 * Depending on the type of warm reset identified, choose the right set
	 * of power management handler and perform the generic, architecture
	 * and platform specific handling.
	 */
	if (psci_get_aff_info_state() == AFF_STATE_ON_PENDING)
		psci_cpu_on_finish(cpu_idx, &state_info);
	else
		psci_cpu_suspend_finish(cpu_idx, &state_info);

	/*
	 * Set the requested and target state of this CPU and all the higher
	 * power domains which are ancestors of this CPU to run.
	 */
	psci_set_pwr_domains_to_run(end_pwrlvl);

	/*
	 * This loop releases the lock corresponding to each power level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl,
				      cpu_idx);
}

/*******************************************************************************
 * This function initializes the set of hooks that PSCI invokes as part of power
 * management operation. The power management hooks are expected to be provided
 * by the SPD, after it finishes all its initialization
 ******************************************************************************/
void psci_register_spd_pm_hook(const spd_pm_ops_t *pm)
{
	assert(pm);
	psci_spd_pm = pm;

	if (pm->svc_migrate)
		psci_caps |= define_psci_cap(PSCI_MIG_AARCH64);

	if (pm->svc_migrate_info)
		psci_caps |= define_psci_cap(PSCI_MIG_INFO_UP_CPU_AARCH64)
				| define_psci_cap(PSCI_MIG_INFO_TYPE);
}

/*******************************************************************************
 * This function invokes the migrate info hook in the spd_pm_ops. It performs
 * the necessary return value validation. If the Secure Payload is UP and
 * migrate capable, it returns the mpidr of the CPU on which the Secure payload
 * is resident through the mpidr parameter. Else the value of the parameter on
 * return is undefined.
 ******************************************************************************/
int psci_spd_migrate_info(u_register_t *mpidr)
{
	int rc;

	if (!psci_spd_pm || !psci_spd_pm->svc_migrate_info)
		return PSCI_E_NOT_SUPPORTED;

	rc = psci_spd_pm->svc_migrate_info(mpidr);

	assert(rc == PSCI_TOS_UP_MIG_CAP || rc == PSCI_TOS_NOT_UP_MIG_CAP \
		|| rc == PSCI_TOS_NOT_PRESENT_MP || rc == PSCI_E_NOT_SUPPORTED);

	return rc;
}


/*******************************************************************************
 * This function prints the state of all power domains present in the
 * system
 ******************************************************************************/
void psci_print_power_domain_map(void)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	unsigned int idx;
	plat_local_state_t state;
	plat_local_state_type_t state_type;

	/* This array maps to the PSCI_STATE_X definitions in psci.h */
	static const char *psci_state_type_str[] = {
		"ON",
		"RETENTION",
		"OFF",
	};

	INFO("PSCI Power Domain Map:\n");
	for (idx = 0; idx < (PSCI_NUM_PWR_DOMAINS - PLATFORM_CORE_COUNT);
							idx++) {
		state_type = find_local_state_type(
				psci_non_cpu_pd_nodes[idx].local_state);
		INFO("  Domain Node : Level %u, parent_node %d,"
				" State %s (0x%x)\n",
				psci_non_cpu_pd_nodes[idx].level,
				psci_non_cpu_pd_nodes[idx].parent_node,
				psci_state_type_str[state_type],
				psci_non_cpu_pd_nodes[idx].local_state);
	}

	for (idx = 0; idx < PLATFORM_CORE_COUNT; idx++) {
		state = psci_get_cpu_local_state_by_idx(idx);
		state_type = find_local_state_type(state);
		INFO("  CPU Node : MPID 0x%lx, parent_node %d,"
				" State %s (0x%x)\n",
				psci_cpu_pd_nodes[idx].mpidr,
				psci_cpu_pd_nodes[idx].parent_node,
				psci_state_type_str[state_type],
				psci_get_cpu_local_state_by_idx(idx));
	}
#endif
}

#if ENABLE_PLAT_COMPAT
/*******************************************************************************
 * PSCI Compatibility helper function to return the 'power_state' parameter of
 * the PSCI CPU SUSPEND request for the current CPU. Returns PSCI_INVALID_DATA
 * if not invoked within CPU_SUSPEND for the current CPU.
 ******************************************************************************/
int psci_get_suspend_powerstate(void)
{
	/* Sanity check to verify that CPU is within CPU_SUSPEND */
	if (psci_get_aff_info_state() == AFF_STATE_ON &&
		!is_local_state_run(psci_get_cpu_local_state()))
		return psci_power_state_compat[plat_my_core_pos()];

	return PSCI_INVALID_DATA;
}

/*******************************************************************************
 * PSCI Compatibility helper function to return the state id of the current
 * cpu encoded in the 'power_state' parameter. Returns PSCI_INVALID_DATA
 * if not invoked within CPU_SUSPEND for the current CPU.
 ******************************************************************************/
int psci_get_suspend_stateid(void)
{
	unsigned int power_state;
	power_state = psci_get_suspend_powerstate();
	if (power_state != PSCI_INVALID_DATA)
		return psci_get_pstate_id(power_state);

	return PSCI_INVALID_DATA;
}

/*******************************************************************************
 * PSCI Compatibility helper function to return the state id encoded in the
 * 'power_state' parameter of the CPU specified by 'mpidr'. Returns
 * PSCI_INVALID_DATA if the CPU is not in CPU_SUSPEND.
 ******************************************************************************/
int psci_get_suspend_stateid_by_mpidr(unsigned long mpidr)
{
	int cpu_idx = plat_core_pos_by_mpidr(mpidr);

	if (cpu_idx == -1)
		return PSCI_INVALID_DATA;

	/* Sanity check to verify that the CPU is in CPU_SUSPEND */
	if (psci_get_aff_info_state_by_idx(cpu_idx) == AFF_STATE_ON &&
		!is_local_state_run(psci_get_cpu_local_state_by_idx(cpu_idx)))
		return psci_get_pstate_id(psci_power_state_compat[cpu_idx]);

	return PSCI_INVALID_DATA;
}

/*******************************************************************************
 * This function returns highest affinity level which is in OFF
 * state. The affinity instance with which the level is associated is
 * determined by the caller.
 ******************************************************************************/
unsigned int psci_get_max_phys_off_afflvl(void)
{
	psci_power_state_t state_info;

	memset(&state_info, 0, sizeof(state_info));
	psci_get_target_local_pwr_states(PLAT_MAX_PWR_LVL, &state_info);

	return psci_find_target_suspend_lvl(&state_info);
}

/*******************************************************************************
 * PSCI Compatibility helper function to return target affinity level requested
 * for the CPU_SUSPEND. This function assumes affinity levels correspond to
 * power domain levels on the platform.
 ******************************************************************************/
int psci_get_suspend_afflvl(void)
{
	return psci_get_suspend_pwrlvl();
}

#endif
