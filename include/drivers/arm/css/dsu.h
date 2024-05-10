/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSU_H
#define DSU_H

#define PMCR_N_MAX			0x1f

#define save_pmu_reg(state, reg) state->reg = read_##reg()

#define restore_pmu_reg(context, reg) write_##reg(context->reg)

typedef struct cluster_pmu_state{
	uint64_t clusterpmcr;
	uint64_t clusterpmcntenset;
	uint64_t clusterpmccntr;
	uint64_t clusterpmovsset;
	uint64_t clusterpmselr;
	uint64_t clusterpmsevtyper;
	uint64_t counter_val[PMCR_N_MAX];
	uint64_t counter_type[PMCR_N_MAX];
} cluster_pmu_state_t;

static inline unsigned int read_cluster_eventctr_num(void)
{
	return ((read_clusterpmcr() >> CLUSTERPMCR_N_SHIFT) &
			CLUSTERPMCR_N_MASK);
}


void save_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_context);

void restore_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_context);

void cluster_on_dsu_pmu_context_restore(void);

void cluster_off_dsu_pmu_context_save(void);

#endif /* DSU_H */
