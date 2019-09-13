/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

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
__section("tzfw_coherent_mem")
#endif
;

/* Lock for PSCI state coordination */
DEFINE_PSCI_LOCK(psci_locks[PSCI_NUM_NON_CPU_PWR_DOMAINS]);

cpu_pd_node_t psci_cpu_pd_nodes[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Pointer to functions exported by the platform to complete power mgmt. ops
 ******************************************************************************/
const plat_psci_ops_t *psci_plat_pm_ops;

/******************************************************************************
 * Check that the maximum power level supported by the platform makes sense
 *****************************************************************************/
CASSERT((PLAT_MAX_PWR_LVL <= PSCI_MAX_PWR_LVL) &&
	(PLAT_MAX_PWR_LVL >= PSCI_CPU_PWR_LVL),
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

/* Function used to categorize plat_local_state. */
static plat_local_state_type_t find_local_state_type(plat_local_state_t state)
{
	if (state != 0U) {
		if (state > PLAT_MAX_RET_STATE) {
			return STATE_TYPE_OFF;
		} else {
			return STATE_TYPE_RETN;
		}
	} else {
		return STATE_TYPE_RUN;
	}
}

/******************************************************************************
 * Check that the maximum retention level supported by the platform is less
 * than the maximum off level.
 *****************************************************************************/
CASSERT(PLAT_MAX_RET_STATE < PLAT_MAX_OFF_STATE,
		assert_platform_max_off_and_retn_state_check);

/******************************************************************************
 * This function ensures that the power state parameter in a CPU_SUSPEND request
 * is valid. If so, it returns the requested states for each power level.
 *****************************************************************************/
int psci_validate_power_state(unsigned int power_state,
			      psci_power_state_t *state_info)
{
	/* Check SBZ bits in power state are zero */
	if (psci_check_power_state(power_state) != 0U)
		return PSCI_E_INVALID_PARAMS;

	assert(psci_plat_pm_ops->validate_power_state != NULL);

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
	assert(psci_plat_pm_ops->get_sys_suspend_power_state != NULL);

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

	for (cpu_idx = 0; cpu_idx < (unsigned int)PLATFORM_CORE_COUNT;
			cpu_idx++) {
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
	assert(pwrlvl < PSCI_INVALID_PWR_LVL);
	return pwrlvl;
}

/******************************************************************************
 * Helper function to update the requested local power state array. This array
 * does not store the requested state for the CPU power level. Hence an
 * assertion is added to prevent us from accessing the CPU power level.
 *****************************************************************************/
static void psci_set_req_local_pwr_state(unsigned int pwrlvl,
					 unsigned int cpu_idx,
					 plat_local_state_t req_pwr_state)
{
	assert(pwrlvl > PSCI_CPU_PWR_LVL);
	if ((pwrlvl > PSCI_CPU_PWR_LVL) && (pwrlvl <= PLAT_MAX_PWR_LVL) &&
			(cpu_idx < (unsigned int) PLATFORM_CORE_COUNT)) {
		psci_req_local_pwr_states[pwrlvl - 1U][cpu_idx] = req_pwr_state;
	}
}

/******************************************************************************
 * This function initializes the psci_req_local_pwr_states.
 *****************************************************************************/
void __init psci_init_req_local_pwr_states(void)
{
	/* Initialize the requested state of all non CPU power domains as OFF */
	unsigned int pwrlvl;
	int core;

	for (pwrlvl = 0U; pwrlvl < PLAT_MAX_PWR_LVL; pwrlvl++) {
		for (core = 0; core < PLATFORM_CORE_COUNT; core++) {
			psci_req_local_pwr_states[pwrlvl][core] =
				PLAT_MAX_OFF_STATE;
		}
	}
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

	if ((pwrlvl > PSCI_CPU_PWR_LVL) && (pwrlvl <= PLAT_MAX_PWR_LVL) &&
			(cpu_idx < (unsigned int) PLATFORM_CORE_COUNT)) {
		return &psci_req_local_pwr_states[pwrlvl - 1U][cpu_idx];
	} else
		return NULL;
}

/*
 * psci_non_cpu_pd_nodes can be placed either in normal memory or coherent
 * memory.
 *
 * With !USE_COHERENT_MEM, psci_non_cpu_pd_nodes is placed in normal memory,
 * it's accessed by both cached and non-cached participants. To serve the common
 * minimum, perform a cache flush before read and after write so that non-cached
 * participants operate on latest data in main memory.
 *
 * When USE_COHERENT_MEM is used, psci_non_cpu_pd_nodes is placed in coherent
 * memory. With HW_ASSISTED_COHERENCY, all PSCI participants are cache-coherent.
 * In both cases, no cache operations are required.
 */

/*
 * Retrieve local state of non-CPU power domain node from a non-cached CPU,
 * after any required cache maintenance operation.
 */
static plat_local_state_t get_non_cpu_pd_node_local_state(
		unsigned int parent_idx)
{
#if !(USE_COHERENT_MEM || HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	flush_dcache_range(
			(uintptr_t) &psci_non_cpu_pd_nodes[parent_idx],
			sizeof(psci_non_cpu_pd_nodes[parent_idx]));
#endif
	return psci_non_cpu_pd_nodes[parent_idx].local_state;
}

/*
 * Update local state of non-CPU power domain node from a cached CPU; perform
 * any required cache maintenance operation afterwards.
 */
static void set_non_cpu_pd_node_local_state(unsigned int parent_idx,
		plat_local_state_t state)
{
	psci_non_cpu_pd_nodes[parent_idx].local_state = state;
#if !(USE_COHERENT_MEM || HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	flush_dcache_range(
			(uintptr_t) &psci_non_cpu_pd_nodes[parent_idx],
			sizeof(psci_non_cpu_pd_nodes[parent_idx]));
#endif
}

/******************************************************************************
 * Helper function to return the current local power state of each power domain
 * from the current cpu power domain to its ancestor at the 'end_pwrlvl'. This
 * function will be called after a cpu is powered on to find the local state
 * each power domain has emerged from.
 *****************************************************************************/
void psci_get_target_local_pwr_states(unsigned int end_pwrlvl,
				      psci_power_state_t *target_state)
{
	unsigned int parent_idx, lvl;
	plat_local_state_t *pd_state = target_state->pwr_domain_state;

	pd_state[PSCI_CPU_PWR_LVL] = psci_get_cpu_local_state();
	parent_idx = psci_cpu_pd_nodes[plat_my_core_pos()].parent_node;

	/* Copy the local power state from node to state_info */
	for (lvl = PSCI_CPU_PWR_LVL + 1U; lvl <= end_pwrlvl; lvl++) {
		pd_state[lvl] = get_non_cpu_pd_node_local_state(parent_idx);
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
	 * Need to flush as local_state might be accessed with Data Cache
	 * disabled during power on
	 */
	psci_flush_cpu_data(psci_svc_cpu_data.local_state);

	parent_idx = psci_cpu_pd_nodes[plat_my_core_pos()].parent_node;

	/* Copy the local_state from state_info */
	for (lvl = 1U; lvl <= end_pwrlvl; lvl++) {
		set_non_cpu_pd_node_local_state(parent_idx, pd_state[lvl]);
		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}
}


/*******************************************************************************
 * PSCI helper function to get the parent nodes corresponding to a cpu_index.
 ******************************************************************************/
void psci_get_parent_pwr_domain_nodes(unsigned int cpu_idx,
				      unsigned int end_lvl,
				      unsigned int *node_index)
{
	unsigned int parent_node = psci_cpu_pd_nodes[cpu_idx].parent_node;
	unsigned int i;
	unsigned int *node = node_index;

	for (i = PSCI_CPU_PWR_LVL + 1U; i <= end_lvl; i++) {
		*node = parent_node;
		node++;
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
	for (lvl = PSCI_CPU_PWR_LVL + 1U; lvl <= end_pwrlvl; lvl++) {
		set_non_cpu_pd_node_local_state(parent_idx,
				PSCI_LOCAL_STATE_RUN);
		psci_set_req_local_pwr_state(lvl,
					     cpu_idx,
					     PSCI_LOCAL_STATE_RUN);
		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

	/* Set the affinity info state to ON */
	psci_set_aff_info_state(AFF_STATE_ON);

	psci_set_cpu_local_state(PSCI_LOCAL_STATE_RUN);
	psci_flush_cpu_data(psci_svc_cpu_data);
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
	unsigned int start_idx;
	unsigned int ncpus;
	plat_local_state_t target_state, *req_states;

	assert(end_pwrlvl <= PLAT_MAX_PWR_LVL);
	parent_idx = psci_cpu_pd_nodes[cpu_idx].parent_node;

	/* For level 0, the requested state will be equivalent
	   to target state */
	for (lvl = PSCI_CPU_PWR_LVL + 1U; lvl <= end_pwrlvl; lvl++) {

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
		if (is_local_state_run(state_info->pwr_domain_state[lvl]) != 0)
			break;

		parent_idx = psci_non_cpu_pd_nodes[parent_idx].parent_node;
	}

	/*
	 * This is for cases when we break out of the above loop early because
	 * the target power state is RUN at a power level < end_pwlvl.
	 * We update the requested power state from state_info and then
	 * set the target state as RUN.
	 */
	for (lvl = lvl + 1U; lvl <= end_pwrlvl; lvl++) {
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

	for (i = (int) target_lvl; i >= (int) PSCI_CPU_PWR_LVL; i--) {
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
	if ((is_power_down_state == 0U) &&
			((max_off_lvl != PSCI_INVALID_PWR_LVL) ||
			 (max_retn_lvl == PSCI_INVALID_PWR_LVL)))
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

	for (i = (int) PLAT_MAX_PWR_LVL; i >= (int) PSCI_CPU_PWR_LVL; i--) {
		if (is_local_state_off(state_info->pwr_domain_state[i]) != 0)
			return (unsigned int) i;
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

	for (i = (int) PLAT_MAX_PWR_LVL; i >= (int) PSCI_CPU_PWR_LVL; i--) {
		if (is_local_state_run(state_info->pwr_domain_state[i]) == 0)
			return (unsigned int) i;
	}

	return PSCI_INVALID_PWR_LVL;
}

/*******************************************************************************
 * This function is passed the highest level in the topology tree that the
 * operation should be applied to and a list of node indexes. It picks up locks
 * from the node index list in order of increasing power domain level in the
 * range specified.
 ******************************************************************************/
void psci_acquire_pwr_domain_locks(unsigned int end_pwrlvl,
				   const unsigned int *parent_nodes)
{
	unsigned int parent_idx;
	unsigned int level;

	/* No locking required for level 0. Hence start locking from level 1 */
	for (level = PSCI_CPU_PWR_LVL + 1U; level <= end_pwrlvl; level++) {
		parent_idx = parent_nodes[level - 1U];
		psci_lock_get(&psci_non_cpu_pd_nodes[parent_idx]);
	}
}

/*******************************************************************************
 * This function is passed the highest level in the topology tree that the
 * operation should be applied to and a list of node indexes. It releases the
 * locks in order of decreasing power domain level in the range specified.
 ******************************************************************************/
void psci_release_pwr_domain_locks(unsigned int end_pwrlvl,
				   const unsigned int *parent_nodes)
{
	unsigned int parent_idx;
	unsigned int level;

	/* Unlock top down. No unlocking required for level 0. */
	for (level = end_pwrlvl; level >= PSCI_CPU_PWR_LVL + 1U; level--) {
		parent_idx = parent_nodes[level - 1U];
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
#ifdef __aarch64__
static int psci_get_ns_ep_info(entry_point_info_t *ep,
			       uintptr_t entrypoint,
			       u_register_t context_id)
{
	u_register_t ep_attr, sctlr;
	unsigned int daif, ee, mode;
	u_register_t ns_scr_el3 = read_scr_el3();
	u_register_t ns_sctlr_el1 = read_sctlr_el1();

	sctlr = ((ns_scr_el3 & SCR_HCE_BIT) != 0U) ?
		read_sctlr_el2() : ns_sctlr_el1;
	ee = 0;

	ep_attr = NON_SECURE | EP_ST_DISABLE;
	if ((sctlr & SCTLR_EE_BIT) != 0U) {
		ep_attr |= EP_EE_BIG;
		ee = 1;
	}
	SET_PARAM_HEAD(ep, PARAM_EP, VERSION_1, ep_attr);

	ep->pc = entrypoint;
	zeromem(&ep->args, sizeof(ep->args));
	ep->args.arg0 = context_id;

	/*
	 * Figure out whether the cpu enters the non-secure address space
	 * in aarch32 or aarch64
	 */
	if ((ns_scr_el3 & SCR_RW_BIT) != 0U) {

		/*
		 * Check whether a Thumb entry point has been provided for an
		 * aarch64 EL
		 */
		if ((entrypoint & 0x1UL) != 0UL)
			return PSCI_E_INVALID_ADDRESS;

		mode = ((ns_scr_el3 & SCR_HCE_BIT) != 0U) ? MODE_EL2 : MODE_EL1;

		ep->spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	} else {

		mode = ((ns_scr_el3 & SCR_HCE_BIT) != 0U) ?
			MODE32_hyp : MODE32_svc;

		/*
		 * TODO: Choose async. exception bits if HYP mode is not
		 * implemented according to the values of SCR.{AW, FW} bits
		 */
		daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

		ep->spsr = SPSR_MODE32(mode, entrypoint & 0x1, ee, daif);
	}

	return PSCI_E_SUCCESS;
}
#else /* !__aarch64__ */
static int psci_get_ns_ep_info(entry_point_info_t *ep,
			       uintptr_t entrypoint,
			       u_register_t context_id)
{
	u_register_t ep_attr;
	unsigned int aif, ee, mode;
	u_register_t scr = read_scr();
	u_register_t ns_sctlr, sctlr;

	/* Switch to non secure state */
	write_scr(scr | SCR_NS_BIT);
	isb();
	ns_sctlr = read_sctlr();

	sctlr = scr & SCR_HCE_BIT ? read_hsctlr() : ns_sctlr;

	/* Return to original state */
	write_scr(scr);
	isb();
	ee = 0;

	ep_attr = NON_SECURE | EP_ST_DISABLE;
	if (sctlr & SCTLR_EE_BIT) {
		ep_attr |= EP_EE_BIG;
		ee = 1;
	}
	SET_PARAM_HEAD(ep, PARAM_EP, VERSION_1, ep_attr);

	ep->pc = entrypoint;
	zeromem(&ep->args, sizeof(ep->args));
	ep->args.arg0 = context_id;

	mode = scr & SCR_HCE_BIT ? MODE32_hyp : MODE32_svc;

	/*
	 * TODO: Choose async. exception bits if HYP mode is not
	 * implemented according to the values of SCR.{AW, FW} bits
	 */
	aif = SPSR_ABT_BIT | SPSR_IRQ_BIT | SPSR_FIQ_BIT;

	ep->spsr = SPSR_MODE32(mode, entrypoint & 0x1, ee, aif);

	return PSCI_E_SUCCESS;
}

#endif /* __aarch64__ */

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
	if (psci_plat_pm_ops->validate_ns_entrypoint != NULL) {
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
void psci_warmboot_entrypoint(void)
{
	unsigned int end_pwrlvl;
	unsigned int cpu_idx = plat_my_core_pos();
	unsigned int parent_nodes[PLAT_MAX_PWR_LVL] = {0};
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

	/* Get the parent nodes */
	psci_get_parent_pwr_domain_nodes(cpu_idx, end_pwrlvl, parent_nodes);

	/*
	 * This function acquires the lock corresponding to each power level so
	 * that by the time all locks are taken, the system topology is snapshot
	 * and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(end_pwrlvl, parent_nodes);

	psci_get_target_local_pwr_states(end_pwrlvl, &state_info);

#if ENABLE_PSCI_STAT
	plat_psci_stat_accounting_stop(&state_info);
#endif

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

#if ENABLE_PSCI_STAT
	/*
	 * Update PSCI stats.
	 * Caches are off when writing stats data on the power down path.
	 * Since caches are now enabled, it's necessary to do cache
	 * maintenance before reading that same data.
	 */
	psci_stats_update_pwr_up(end_pwrlvl, &state_info);
#endif

	/*
	 * This loop releases the lock corresponding to each power level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl, parent_nodes);
}

/*******************************************************************************
 * This function initializes the set of hooks that PSCI invokes as part of power
 * management operation. The power management hooks are expected to be provided
 * by the SPD, after it finishes all its initialization
 ******************************************************************************/
void psci_register_spd_pm_hook(const spd_pm_ops_t *pm)
{
	assert(pm != NULL);
	psci_spd_pm = pm;

	if (pm->svc_migrate != NULL)
		psci_caps |= define_psci_cap(PSCI_MIG_AARCH64);

	if (pm->svc_migrate_info != NULL)
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

	if ((psci_spd_pm == NULL) || (psci_spd_pm->svc_migrate_info == NULL))
		return PSCI_E_NOT_SUPPORTED;

	rc = psci_spd_pm->svc_migrate_info(mpidr);

	assert((rc == PSCI_TOS_UP_MIG_CAP) || (rc == PSCI_TOS_NOT_UP_MIG_CAP) ||
	       (rc == PSCI_TOS_NOT_PRESENT_MP) || (rc == PSCI_E_NOT_SUPPORTED));

	return rc;
}


/*******************************************************************************
 * This function prints the state of all power domains present in the
 * system
 ******************************************************************************/
void psci_print_power_domain_map(void)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	int idx;
	plat_local_state_t state;
	plat_local_state_type_t state_type;

	/* This array maps to the PSCI_STATE_X definitions in psci.h */
	static const char * const psci_state_type_str[] = {
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
		INFO("  CPU Node : MPID 0x%llx, parent_node %d,"
				" State %s (0x%x)\n",
				(unsigned long long)psci_cpu_pd_nodes[idx].mpidr,
				psci_cpu_pd_nodes[idx].parent_node,
				psci_state_type_str[state_type],
				psci_get_cpu_local_state_by_idx(idx));
	}
#endif
}

/******************************************************************************
 * Return whether any secondaries were powered up with CPU_ON call. A CPU that
 * have ever been powered up would have set its MPDIR value to something other
 * than PSCI_INVALID_MPIDR. Note that MPDIR isn't reset back to
 * PSCI_INVALID_MPIDR when a CPU is powered down later, so the return value is
 * meaningful only when called on the primary CPU during early boot.
 *****************************************************************************/
int psci_secondaries_brought_up(void)
{
	unsigned int idx, n_valid = 0U;

	for (idx = 0U; idx < ARRAY_SIZE(psci_cpu_pd_nodes); idx++) {
		if (psci_cpu_pd_nodes[idx].mpidr != PSCI_INVALID_MPIDR)
			n_valid++;
	}

	assert(n_valid > 0U);

	return (n_valid > 1U) ? 1 : 0;
}

/*******************************************************************************
 * Initiate power down sequence, by calling power down operations registered for
 * this CPU.
 ******************************************************************************/
void psci_do_pwrdown_sequence(unsigned int power_level)
{
#if HW_ASSISTED_COHERENCY
	/*
	 * With hardware-assisted coherency, the CPU drivers only initiate the
	 * power down sequence, without performing cache-maintenance operations
	 * in software. Data caches enabled both before and after this call.
	 */
	prepare_cpu_pwr_dwn(power_level);
#else
	/*
	 * Without hardware-assisted coherency, the CPU drivers disable data
	 * caches, then perform cache-maintenance operations in software.
	 *
	 * This also calls prepare_cpu_pwr_dwn() to initiate power down
	 * sequence, but that function will return with data caches disabled.
	 * We must ensure that the stack memory is flushed out to memory before
	 * we start popping from it again.
	 */
	psci_do_pwrdown_cache_maintenance(power_level);
#endif
}
