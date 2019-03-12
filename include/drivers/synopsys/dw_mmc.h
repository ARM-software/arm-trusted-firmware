/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DW_MMC_H
#define DW_MMC_H

#include <drivers/mmc.h>

typedef struct dw_mmc_params {
	uintptr_t	reg_base;
	uintptr_t	desc_base;
	size_t		desc_size;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
	enum mmc_device_type	mmc_dev_type;
} dw_mmc_params_t;

void dw_mmc_init(dw_mmc_params_t *params, struct mmc_device_info *info);

#endif /* DW_MMC_H */
