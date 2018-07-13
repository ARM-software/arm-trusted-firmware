/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STM32MP1_DT_H__
#define __STM32MP1_DT_H__

#include <stdbool.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
int dt_open_and_check(void);
int fdt_get_address(void **fdt_addr);
bool fdt_check_node(int node);
bool fdt_check_status(int node);
bool fdt_check_secure_status(int node);
uint32_t fdt_read_uint32_default(int node, const char *prop_name,
				 uint32_t dflt_value);
int fdt_read_uint32_array(int node, const char *prop_name,
			  uint32_t *array, uint32_t count);
int dt_set_pinctrl_config(int node);

#endif /* __STM32MP1_DT_H__ */
