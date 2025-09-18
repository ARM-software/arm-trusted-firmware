/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DSU_H
#define DSU_H

#if defined(__aarch64__)
#include <dsu_def.h>

/*
 * Power Control Registers enable bit of Auxilary Control register.
 * ACTLR_EL3_PWREN_BIT definition is same among cores like Cortex-X925,
 * Cortex-X4, Cortex-A520, Cortex-A725 that are used in a cluster
 * with DSU.
 */
#define ACTLR_EL3_PWREN_BIT		BIT(7)

/* PMU Registers enable bit of Auxilary Control register of EL3 and EL2. */
#define ACTLR_CLUSTERPMUEN		BIT(12)

#define PMCR_N_MAX			0x1f

#define save_pmu_reg(state, reg) state->reg = read_##reg()

#define restore_pmu_reg(context, reg) write_##reg(context->reg)

typedef struct cluster_pmu_state {
	uint64_t clusterpmcr;
	uint64_t clusterpmcntenset;
	uint64_t clusterpmccntr;
	uint64_t clusterpmovsset;
	uint64_t clusterpmselr;
	uint64_t clusterpmsevtyper;
	uint64_t counter_val[PMCR_N_MAX];
	uint64_t counter_type[PMCR_N_MAX];
} cluster_pmu_state_t;

typedef struct dsu_driver_data {
	uint8_t clusterpwrdwn_pwrdn;
	uint8_t clusterpwrdwn_memret;
	uint8_t clusterpwrctlr_cachepwr;
	uint8_t clusterpwrctlr_funcret;
} dsu_driver_data_t;

extern const dsu_driver_data_t plat_dsu_data;

static inline unsigned int read_cluster_eventctr_num(void)
{
	return ((read_clusterpmcr() >> CLUSTERPMCR_N_SHIFT) &
			CLUSTERPMCR_N_MASK);
}

void save_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_context);

void restore_dsu_pmu_state(cluster_pmu_state_t *cluster_pmu_context);

void cluster_on_dsu_pmu_context_restore(void);

void cluster_off_dsu_pmu_context_save(void);

void dsu_driver_init(const dsu_driver_data_t *data);
#endif
#endif /* DSU_H */
