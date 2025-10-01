/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef S32CC_CLK_DRV_H
#define S32CC_CLK_DRV_H

int s32cc_init_core_clocks(void);
int s32cc_init_early_clks(void);
int plat_deassert_ddr_reset(void);

#endif
