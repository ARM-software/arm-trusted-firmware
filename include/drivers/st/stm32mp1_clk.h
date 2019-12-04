/*
 * Copyright (c) 2018-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_CLK_H
#define STM32MP1_CLK_H

#include <arch_helpers.h>

enum stm32mp_osc_id {
	_HSI,
	_HSE,
	_CSI,
	_LSI,
	_LSE,
	_I2S_CKIN,
	NB_OSC,
	_UNKNOWN_OSC_ID = 0xFF
};

extern const char *stm32mp_osc_node_label[NB_OSC];

int stm32mp1_clk_probe(void);
int stm32mp1_clk_init(void);

bool stm32mp1_rcc_is_secure(void);
bool stm32mp1_rcc_is_mckprot(void);

/* SMP protection on RCC registers access */
void stm32mp1_clk_rcc_regs_lock(void);
void stm32mp1_clk_rcc_regs_unlock(void);

#ifdef STM32MP_SHARED_RESOURCES
void stm32mp1_register_clock_parents_secure(unsigned long id);
#endif
#endif /* STM32MP1_CLK_H */
