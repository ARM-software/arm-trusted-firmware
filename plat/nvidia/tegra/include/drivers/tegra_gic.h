/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_GIC_H__
#define __TEGRA_GIC_H__

#include <common/interrupt_props.h>

/*******************************************************************************
 * Per-CPU struct describing FIQ state to be stored
 ******************************************************************************/
typedef struct pcpu_fiq_state {
	uint64_t elr_el3;
	uint64_t spsr_el3;
} pcpu_fiq_state_t;

/*******************************************************************************
 * Fucntion declarations
 ******************************************************************************/
void tegra_gic_cpuif_deactivate(void);
void tegra_gic_init(void);
void tegra_gic_pcpu_init(void);
void tegra_gic_setup(const interrupt_prop_t *interrupt_props,
		     unsigned int interrupt_props_num);

#endif /* __TEGRA_GIC_H__ */
