/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_CLKFUNC_H
#define STM32MP_CLKFUNC_H

#include <stdbool.h>

#include <libfdt.h>

int fdt_get_rcc_node(void *fdt);
uint32_t fdt_rcc_read_addr(void);
int fdt_rcc_read_uint32_array(const char *prop_name,
			      uint32_t *array, uint32_t count);
int fdt_rcc_subnode_offset(const char *name);
const fdt32_t *fdt_rcc_read_prop(const char *prop_name, int *lenp);
bool fdt_get_rcc_secure_status(void);

uintptr_t fdt_get_stgen_base(void);
int fdt_get_clock_id(int node);

#endif /* STM32MP_CLKFUNC_H */
