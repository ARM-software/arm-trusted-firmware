/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_SDMMC2_H
#define STM32_SDMMC2_H

#include <stdbool.h>

#include <drivers/mmc.h>

struct stm32_sdmmc2_params {
	uintptr_t		reg_base;
	unsigned int		clk_rate;
	unsigned int		bus_width;
	unsigned int		flags;
	struct mmc_device_info	*device_info;
	unsigned int		pin_ckin;
	unsigned int		negedge;
	unsigned int		dirpol;
	unsigned int		clock_id;
	unsigned int		reset_id;
	unsigned int		max_freq;
	bool			use_dma;
};

unsigned long long stm32_sdmmc2_mmc_get_device_size(void);
int stm32_sdmmc2_mmc_init(struct stm32_sdmmc2_params *params);
bool plat_sdmmc2_use_dma(unsigned int instance, unsigned int memory);

#endif /* STM32_SDMMC2_H */
