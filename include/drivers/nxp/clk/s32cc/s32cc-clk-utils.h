/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */
#ifndef S32CC_CLK_UTILS_H
#define S32CC_CLK_UTILS_H

#include <stdbool.h>
#include <s32cc-clk-modules.h>

struct s32cc_clk *s32cc_get_clk_from_table(const struct s32cc_clk_array *const *clk_arr,
					   size_t size,
					   unsigned long clk_id);

int s32cc_get_id_from_table(const struct s32cc_clk_array *const *clk_arr,
			    size_t size, const struct s32cc_clk *clk,
			    unsigned long *clk_index);

struct s32cc_clk *s32cc_get_arch_clk(unsigned long id);
int s32cc_get_clk_id(const struct s32cc_clk *clk, unsigned long *id);

int s32cc_clk_register_drv(bool mmap_regs);

#endif /* S32CC_CLK_UTILS_H */
