/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <errata_report.h>
#include <platform.h>
#include <stddef.h>
#include "psci_private.h"

/*******************************************************************************
 * Per cpu non-secure contexts used to program the architectural state prior
 * return to the normal world.
 * TODO: Use the memory allocator to set aside memory for the contexts instead
 * of relying on platform defined constants.
 ******************************************************************************/
static cpu_context_t psci_ns_context[PLATFORM_CORE_COUNT];

/******************************************************************************
 * Define the psci capability variable.
 *****************************************************************************/
unsigned int psci_caps;

/*******************************************************************************
 * Function which initializes the 'psci_non_cpu_pd_nodes' or the
 * 'psci_cpu_pd_nodes' corresponding to the power level.
 ******************************************************************************/
static void __init psci_init_pwr_domain_node(unsigned char node_idx,
					unsigned int parent_idx,
					unsigned char level)
{
	if (level > PSCI_CPU_PWR_LVL) {
		psci_non_cpu_pd_nodes[node_idx].level = level;
		psci_lock_init(psci_non_cpu_pd_nodes, node_idx);
		psci_non_cpu_pd_nodes[node_idx].parent_node = parent_idx;
		psci_non_cpu_pd_nodes[node_idx].local_state =
							 PLAT_MAX_OFF_STATE;
	} else {
		psci_cpu_data_t *svc_cpu_data;

		psci_cpu_pd_nodes[node_idx].parent_node = parent_idx;

		/* Initialize with an invalid mpidr */
		psci_cpu_pd_nodes[node_idx].mpidr = PSCI_INVALID_MPIDR;

		svc_cpu_data =
			&(_cpu_data_by_index(node_idx)->psci_svc_cpu_data);

		/* Set the Affinity Info for the cores as OFF */
		svc_cpu_data->aff_info_state = AFF_STATE_OFF;

		/* Invalidate the suspend level for the cpu */
		svc_cpu_data->target_pwrlvl = PSCI_INVALID_PWR_LVL;

		/* Set the power state to OFF state */
		svc_cpu_data->local_state = PLAT_MAX_OFF_STATE;

		psci_flush_dcache_range((uintptr_t)svc_cpu_data,
						 sizeof(*svc_cpu_data));

		cm_set_context_by_index(node_idx,
					(void *) &psci_ns_context[node_idx],
					NON_SECURE);
	}
}

/*******************************************************************************
 * This functions updates cpu_start_idx and ncpus field for each of the node in
 * psci_non_cpu_pd_nodes[]. It does so by comparing the parent nodes of each of
 * the CPUs and check whether they match with the parent of the previous
 * CPU. The basic assumption for this work is that children of the same parent
 * are allocated adjacent indices. The platform should ensure this though proper
 * mapping of the CPUs to indices via plat_core_pos_by_mpidr() and
 * plat_my_core_pos() APIs.
 *******************************************************************************/
static void __init psci_update_pwrlvl_limits(void)
{
	int j, cpu_idx;
	unsigned int nodes_idx[PLAT_MAX_PWR_LVL] = {0};
	unsigned int temp_index[PLAT_MAX_PWR_LVL];

	for (cpu_idx = 0; cpu_idx < PLATFORM_CORE_COUNT; cpu_idx++) {
		psci_get_parent_pwr_domain_nodes(cpu_idx,
						 (unsigned int)PLAT_MAX_PWR_LVL,
						 temp_index);
		for (j = (int) PLAT_MAX_PWR_LVL - 1; j >= 0; j--) {
			if (temp_index[j] != nodes_idx[j]) {
				nodes_idx[j] = temp_index[j];
				psci_non_cpu_pd_nodes[nodes_idx[j]].cpu_start_idx
					= cpu_idx;
			}
			psci_non_cpu_pd_nodes[nodes_idx[j]].ncpus++;
		}
	}
}

/*******************************************************************************
 * Core routine to populate the power domain tree. The tree descriptor passed by
 * the platform is populated breadth-first and the first entry in the map
 * informs the number of root power domains. The parent nodes of the root nodes
 * will point to an invalid entry(-1).
 ******************************************************************************/
static void __init populate_power_domain_tree(const unsigned char *topology)
{
	unsigned int i, j = 0U, num_nodes_at_lvl = 1U, num_nodes_at_next_lvl;
	unsigned int node_index = 0U, num_children;
	int parent_node_index = 0;
	int level = (int) PLAT_MAX_PWR_LVL;

	/*
	 * For each level the inputs are:
	 * - number of nodes at this level in plat_array i.e. num_nodes_at_level
	 *   This is the sum of values of nodes at the parent level.
	 * - Index of first entry at this level in the plat_array i.e.
	 *   parent_node_index.
	 * - Index of first free entry in psci_non_cpu_pd_nodes[] or
	 *   psci_cpu_pd_nodes[] i.e. node_index depending upon the level.
	 */
	while (level >= (int) PSCI_CPU_PWR_LVL) {
		num_nodes_at_next_lvl = 0U;
		/*
		 * For each entry (parent node) at this level in the plat_array:
		 * - Find the number of children
		 * - Allocate a node in a power domain array for each child
		 * - Set the parent of the child to the parent_node_index - 1
		 * - Increment parent_node_index to point to the next parent
		 * - Accumulate the number of children at next level.
		 */
		for (i = 0U; i < num_nodes_at_lvl; i++) {
			assert(parent_node_index <=
					PSCI_NUM_NON_CPU_PWR_DOMAINS);
			num_children = topology[parent_node_index];

			for (j = node_index;
				j < (node_index + num_children); j++)
				psci_init_pwr_domain_node((unsigned char)j,
							  parent_node_index - 1,
							  (unsigned char)level);

			node_index = j;
			num_nodes_at_next_lvl += num_children;
			parent_node_index++;
		}

		num_nodes_at_lvl = num_nodes_at_next_lvl;
		level--;

		/* Reset the index for the cpu power domain array */
		if (level == (int) PSCI_CPU_PWR_LVL)
			node_index = 0;
	}

	/* Validate the sanity of array exported by the platform */
	assert((int) j == PLATFORM_CORE_COUNT);
}

/*******************************************************************************
 * This function does the architectural setup and takes the warm boot
 * entry-point `mailbox_ep` as an argument. The function also initializes the
 * power domain topology tree by querying the platform. The power domain nodes
 * higher than the CPU are populated in the array psci_non_cpu_pd_nodes[] and
 * the CPU power domains are populated in psci_cpu_pd_nodes[]. The platform
 * exports its static topology map through the
 * populate_power_domain_topology_tree() API. The algorithm populates the
 * psci_non_cpu_pd_nodes and psci_cpu_pd_nodes iteratively by using this
 * topology map.  On a platform that implements two clusters of 2 cpus each,
 * and supporting 3 domain levels, the populated psci_non_cpu_pd_nodes would
 * look like this:
 *
 * ---------------------------------------------------
 * | system node | cluster 0 node  | cluster 1 node  |
 * ---------------------------------------------------
 *
 * And populated psci_cpu_pd_nodes would look like this :
 * <-    cpus cluster0   -><-   cpus cluster1   ->
 * ------------------------------------------------
 * |   CPU 0   |   CPU 1   |   CPU 2   |   CPU 3  |
 * ------------------------------------------------
 ******************************************************************************/
int __init psci_setup(const psci_lib_args_t *lib_args)
{
	const unsigned char *topology_tree;

	assert(VERIFY_PSCI_LIB_ARGS_V1(lib_args));

	/* Do the Architectural initialization */
	psci_arch_setup();

	/* Query the topology map from the platform */
	topology_tree = plat_get_power_domain_tree_desc();

	/* Populate the power domain arrays using the platform topology map */
	populate_power_domain_tree(topology_tree);

	/* Update the CPU limits for each node in psci_non_cpu_pd_nodes */
	psci_update_pwrlvl_limits();

	/* Populate the mpidr field of cpu node for this CPU */
	psci_cpu_pd_nodes[plat_my_core_pos()].mpidr =
		read_mpidr() & MPIDR_AFFINITY_MASK;

	psci_init_req_local_pwr_states();

	/*
	 * Set the requested and target state of this CPU and all the higher
	 * power domain levels for this CPU to run.
	 */
	psci_set_pwr_domains_to_run(PLAT_MAX_PWR_LVL);

	(void) plat_setup_psci_ops((uintptr_t)lib_args->mailbox_ep,
				   &psci_plat_pm_ops);
	assert(psci_plat_pm_ops != NULL);

	/*
	 * Flush `psci_plat_pm_ops` as it will be accessed by secondary CPUs
	 * during warm boot, possibly before data cache is enabled.
	 */
	psci_flush_dcache_range((uintptr_t)&psci_plat_pm_ops,
					sizeof(psci_plat_pm_ops));

	/* Initialize the psci capability */
	psci_caps = PSCI_GENERIC_CAP;

	if (psci_plat_pm_ops->pwr_domain_off != NULL)
		psci_caps |=  define_psci_cap(PSCI_CPU_OFF);
	if ((psci_plat_pm_ops->pwr_domain_on != NULL) &&
	    (psci_plat_pm_ops->pwr_domain_on_finish != NULL))
		psci_caps |=  define_psci_cap(PSCI_CPU_ON_AARCH64);
	if ((psci_plat_pm_ops->pwr_domain_suspend != NULL) &&
	    (psci_plat_pm_ops->pwr_domain_suspend_finish != NULL)) {
		psci_caps |=  define_psci_cap(PSCI_CPU_SUSPEND_AARCH64);
		if (psci_plat_pm_ops->get_sys_suspend_power_state != NULL)
			psci_caps |=  define_psci_cap(PSCI_SYSTEM_SUSPEND_AARCH64);
	}
	if (psci_plat_pm_ops->system_off != NULL)
		psci_caps |=  define_psci_cap(PSCI_SYSTEM_OFF);
	if (psci_plat_pm_ops->system_reset != NULL)
		psci_caps |=  define_psci_cap(PSCI_SYSTEM_RESET);
	if (psci_plat_pm_ops->get_node_hw_state != NULL)
		psci_caps |= define_psci_cap(PSCI_NODE_HW_STATE_AARCH64);
	if ((psci_plat_pm_ops->read_mem_protect != NULL) &&
			(psci_plat_pm_ops->write_mem_protect != NULL))
		psci_caps |= define_psci_cap(PSCI_MEM_PROTECT);
	if (psci_plat_pm_ops->mem_protect_chk != NULL)
		psci_caps |= define_psci_cap(PSCI_MEM_CHK_RANGE_AARCH64);
	if (psci_plat_pm_ops->system_reset2 != NULL)
		psci_caps |= define_psci_cap(PSCI_SYSTEM_RESET2_AARCH64);

#if ENABLE_PSCI_STAT
	psci_caps |=  define_psci_cap(PSCI_STAT_RESIDENCY_AARCH64);
	psci_caps |=  define_psci_cap(PSCI_STAT_COUNT_AARCH64);
#endif

	return 0;
}

/*******************************************************************************
 * This duplicates what the primary cpu did after a cold boot in BL1. The same
 * needs to be done when a cpu is hotplugged in. This function could also over-
 * ride any EL3 setup done by BL1 as this code resides in rw memory.
 ******************************************************************************/
void psci_arch_setup(void)
{
#if (ARM_ARCH_MAJOR > 7) || defined(ARMV7_SUPPORTS_GENERIC_TIMER)
	/* Program the counter frequency */
	write_cntfrq_el0(plat_get_syscnt_freq2());
#endif

	/* Initialize the cpu_ops pointer. */
	init_cpu_ops();

	/* Having initialized cpu_ops, we can now print errata status */
	print_errata_status();
}

/******************************************************************************
 * PSCI Library interface to initialize the cpu context for the next non
 * secure image during cold boot. The relevant registers in the cpu context
 * need to be retrieved and programmed on return from this interface.
 *****************************************************************************/
void psci_prepare_next_non_secure_ctx(entry_point_info_t *next_image_info)
{
	assert(GET_SECURITY_STATE(next_image_info->h.attr) == NON_SECURE);
	cm_init_my_context(next_image_info);
	cm_prepare_el3_exit(NON_SECURE);
}
