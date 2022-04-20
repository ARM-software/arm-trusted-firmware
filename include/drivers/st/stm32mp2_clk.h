/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_CLK_H
#define STM32MP2_CLK_H

#include <platform_def.h>

enum stm32mp_osc_id {
	_HSI,
	_HSE,
	_CSI,
	_LSI,
	_LSE,
	_I2S_CKIN,
	_SPDIF_SYMB,
	NB_OSC,
	_UNKNOWN_OSC_ID = 0xFF
};

extern const char *stm32mp_osc_node_label[NB_OSC];

enum pll_cfg {
	FBDIV,
	REFDIV,
	POSTDIV1,
	POSTDIV2,
	PLLCFG_NB
};

enum pll_csg {
	DIVVAL,
	SPREAD,
	DOWNSPREAD,
	PLLCSG_NB
};

int stm32mp2_clk_init(void);
int stm32mp2_pll1_disable(void);

#endif /* STM32MP2_CLK_H */
