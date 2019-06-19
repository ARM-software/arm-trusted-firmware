/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
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

void __stm32mp1_clk_enable(unsigned long id, bool caller_is_secure);
void __stm32mp1_clk_disable(unsigned long id, bool caller_is_secure);

static inline void stm32mp1_clk_enable_non_secure(unsigned long id)
{
	__stm32mp1_clk_enable(id, false);
}

static inline void stm32mp1_clk_enable_secure(unsigned long id)
{
	__stm32mp1_clk_enable(id, true);
}

static inline void stm32mp1_clk_disable_non_secure(unsigned long id)
{
	__stm32mp1_clk_disable(id, false);
}

static inline void stm32mp1_clk_disable_secure(unsigned long id)
{
	__stm32mp1_clk_disable(id, true);
}

unsigned int stm32mp1_clk_get_refcount(unsigned long id);

/* SMP protection on RCC registers access */
void stm32mp1_clk_rcc_regs_lock(void);
void stm32mp1_clk_rcc_regs_unlock(void);

void stm32mp1_stgen_increment(unsigned long long offset_in_ms);

#endif /* STM32MP1_CLK_H */
