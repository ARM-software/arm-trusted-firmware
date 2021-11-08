/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUPWR_CLKCTL_H
#define APUPWR_CLKCTL_H

#include <arch_helpers.h>
#include <apupwr_clkctl_def.h>

int32_t apupwr_smc_acc_init_all(void);
void apupwr_smc_acc_top(bool enable);
int32_t apupwr_smc_acc_set_parent(uint32_t freq, uint32_t domain);
int32_t apupwr_smc_pll_set_rate(uint32_t pll, bool div2, uint32_t domain);
int32_t apupwr_smc_bulk_pll(bool enable);
void apupwr_smc_bus_prot_cg_on(void);

int32_t apu_pll_enable(int32_t pll_idx, bool enable, bool fhctl_en);
int32_t anpu_pll_set_rate(enum dvfs_voltage_domain domain,
			  enum pll_set_rate_mode mode, int32_t freq);
#endif /* APUPWR_CLKCTL_H */
