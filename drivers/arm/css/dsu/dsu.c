/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/dsu.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/*
 * Context structure that saves the state of DSU PMU registers
 */
cluster_pmu_state_t cluster_pmu_context[PLAT_ARM_CLUSTER_COUNT];

/****************************************************************************
 * This function, save_dsu_pmu_state, is designed to save the
 * current state of the Performance Monitoring Unit (PMU) for a cluster.
 *
 * The function performs the following operations:
 * 1. Saves the current values of several PMU registers
 *    (CLUSTERPMCR_EL1, CLUSTERPMCNTENSET_EL1, CLUSTERPMCCNTR_EL1,
 *    CLUSTERPMOVSSET_EL1, and CLUSTERPMSELR_EL1) into the cluster_pmu_state
 *    structure.
 *
 * 2. Disables the PMU event counting by
 *    clearing the E bit in the clusterpmcr_el1 register.
 *
 * 3. Iterates over the available PMU counters as
 *    determined by the read_cluster_eventctr_num() function.
 *    For each counter, it:
 *    a. Selects the counter by writing its index to CLUSTERPMSELR_EL1.
 *    b. Reads the current counter value (event count) and
 *       the event type being counted from CLUSTERPMXEVCNTR_EL1 and
 *       CLUSTERPMXEVTYPER_EL1 registers, respectively.
 *
 * This function is useful for preserving the DynamIQ Shared Unit's (DSU)
 * PMU registers over a power cycle.
 ***************************************************************************/

void save_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_state)
{
	unsigned int idx = 0U;
	unsigned int cluster_eventctr_num = read_cluster_eventctr_num();

	assert(cluster_pmu_state != 0);

	save_pmu_reg(cluster_pmu_state, clusterpmcr);

	write_clusterpmcr(cluster_pmu_state->clusterpmcr &
			~(CLUSTERPMCR_E_BIT));

	save_pmu_reg(cluster_pmu_state, clusterpmcntenset);

	save_pmu_reg(cluster_pmu_state, clusterpmccntr);

	save_pmu_reg(cluster_pmu_state, clusterpmovsset);

	save_pmu_reg(cluster_pmu_state, clusterpmselr);

	for (idx = 0U ; idx < cluster_eventctr_num ; idx++) {
		write_clusterpmselr(idx);
		cluster_pmu_state->counter_val[idx] = read_clusterpmxevcntr();
		cluster_pmu_state->counter_type[idx] = read_clusterpmxevtyper();
	}
}

void cluster_off_dsu_pmu_context_save(void)
{
	unsigned int cluster_pos;

	cluster_pos = (unsigned int) plat_cluster_id_by_mpidr(read_mpidr_el1());

	save_dsu_pmu_state(&cluster_pmu_context[cluster_pos]);
}

/*****************************************************************************
 * This function, restore_dsu_pmu_state, restores the state of the
 * Performance Monitoring Unit (PMU) from a previously saved state.
 *
 * The function performs the following operations:
 * 1. Restores the CLUSTERPMCR_EL1 register with the
 *    saved value from the cluster_pmu_state structure.
 * 2. Iterates over the available PMU counters as determined
 *    by the read_cluster_eventctr_num() function. For each counter, it:
 *    a. Selects the counter by writing its index to CLUSTERPMSELR_EL1.
 *    b. Restores the counter value (event count) and the event type to
 *       CLUSTERPMXEVCNTR_EL1 and CLUSTERPMXEVTYPER_EL1 registers, respectively
 * 3. Restores several other PMU registers (CLUSTERPMSELR_EL1,
 *    CLUSTERPMOVSCLR_EL1, CLUSTERPMOVSSET_EL1, CLUSTERPMCCNTR_EL1,
 *    and CLUSTERPMCNTENSET_EL1) with their saved values.
 *
 *****************************************************************************/
void restore_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_state)
{
	unsigned int idx = 0U;
	unsigned int cluster_eventctr_num = read_cluster_eventctr_num();

	assert(cluster_pmu_state != 0);

	for (idx = 0U ; idx < cluster_eventctr_num ; idx++) {
		write_clusterpmselr(idx);
		write_clusterpmxevcntr(cluster_pmu_state->counter_val[idx]);
		write_clusterpmxevtyper(cluster_pmu_state->counter_type[idx]);
	}

	restore_pmu_reg(cluster_pmu_state, clusterpmselr);

	write_clusterpmovsclr(~(uint32_t)cluster_pmu_state->clusterpmovsset);

	restore_pmu_reg(cluster_pmu_state, clusterpmovsset);

	restore_pmu_reg(cluster_pmu_state, clusterpmccntr);

	restore_pmu_reg(cluster_pmu_state, clusterpmcntenset);

	write_clusterpmcr(cluster_pmu_state->clusterpmcr);
}

void cluster_on_dsu_pmu_context_restore(void)
{
	unsigned int cluster_pos;

	cluster_pos = (unsigned int) plat_cluster_id_by_mpidr(read_mpidr_el1());

	restore_dsu_pmu_state(&cluster_pmu_context[cluster_pos]);
}

