/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_CLKFUNC_H
#define STM32MP1_CLKFUNC_H

#include <stdbool.h>

#include <libfdt.h>

enum stm32mp_osc_id {
	_HSI,
	_HSE,
	_CSI,
	_LSI,
	_LSE,
	_I2S_CKIN,
	_USB_PHY_48,
	NB_OSC,
	_UNKNOWN_OSC_ID = 0xFF
};

extern const char *stm32mp_osc_node_label[NB_OSC];

int fdt_osc_read_freq(const char *name, uint32_t *freq);
bool fdt_osc_read_bool(enum stm32mp_osc_id osc_id, const char *prop_name);
uint32_t fdt_osc_read_uint32_default(enum stm32mp_osc_id osc_id,
				     const char *prop_name,
				     uint32_t dflt_value);

uint32_t fdt_rcc_read_addr(void);
int fdt_rcc_read_uint32_array(const char *prop_name,
			      uint32_t *array, uint32_t count);
int fdt_rcc_subnode_offset(const char *name);
const fdt32_t *fdt_rcc_read_prop(const char *prop_name, int *lenp);
bool fdt_get_rcc_secure_status(void);

uintptr_t fdt_get_stgen_base(void);
int fdt_get_clock_id(int node);

#endif /* STM32MP1_CLKFUNC_H */
