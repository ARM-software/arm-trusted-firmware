/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DW_MMC_H__
#define __DW_MMC_H__

typedef struct dw_mmc_params {
	uintptr_t	reg_base;
	uintptr_t	desc_base;
	size_t		desc_size;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
} dw_mmc_params_t;

void dw_mmc_init(dw_mmc_params_t *params);

#endif	/* __DW_MMC_H__ */
