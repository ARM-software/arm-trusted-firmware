/*
 * Copyright (c) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_CLK_H
#define STM32MP1_CLK_H

#include <stdbool.h>

#include <arch_helpers.h>

int stm32mp1_clk_probe(void);
int stm32mp1_clk_init(void);
bool stm32mp1_clk_is_enabled(unsigned long id);
int stm32mp1_clk_enable(unsigned long id);
int stm32mp1_clk_disable(unsigned long id);
unsigned long stm32mp1_clk_get_rate(unsigned long id);
void stm32mp1_stgen_increment(unsigned long long offset_in_ms);

static inline uint32_t get_timer(uint32_t base)
{
	if (base == 0U) {
		return (uint32_t)(~read_cntpct_el0());
	}

	return base - (uint32_t)(~read_cntpct_el0());
}

#endif /* STM32MP1_CLK_H */
