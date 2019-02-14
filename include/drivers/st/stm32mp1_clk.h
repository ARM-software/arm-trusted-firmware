/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_CLK_H
#define STM32MP1_CLK_H

#include <arch_helpers.h>

int stm32mp1_clk_probe(void);
int stm32mp1_clk_init(void);
void stm32mp1_stgen_increment(unsigned long long offset_in_ms);

#endif /* STM32MP1_CLK_H */
