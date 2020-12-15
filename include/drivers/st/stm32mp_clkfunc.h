/*
 * Copyright (c) 2017-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_CLKFUNC_H
#define STM32MP_CLKFUNC_H

#include <stdbool.h>

#include <libfdt.h>

#include <platform_def.h>

int fdt_osc_read_freq(const char *name, uint32_t *freq);
bool fdt_clk_read_bool(const char *node_label, const char *prop_name);
uint32_t fdt_clk_read_uint32_default(const char *node_label,
				     const char *prop_name,
				     uint32_t dflt_value);

int fdt_rcc_read_uint32_array(const char *prop_name, uint32_t count,
			      uint32_t *array);
int fdt_rcc_subnode_offset(const char *name);
const fdt32_t *fdt_rcc_read_prop(const char *prop_name, int *lenp);
bool fdt_get_rcc_secure_state(void);

int fdt_get_clock_id(int node);
unsigned long fdt_get_uart_clock_freq(uintptr_t instance);

void stm32mp_stgen_config(unsigned long rate);
void stm32mp_stgen_restore_counter(unsigned long long value,
				   unsigned long long offset_in_ms);
unsigned long long stm32mp_stgen_get_counter(void);

#endif /* STM32MP_CLKFUNC_H */
