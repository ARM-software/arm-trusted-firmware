/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STM32MP1_DT_H__
#define __STM32MP1_DT_H__

#include <stdbool.h>

struct dt_node_info {
	uint32_t base;
	int32_t clock;
	int32_t reset;
	bool status;
	bool sec_status;
};

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
int dt_set_stdout_pinctrl(void);
void dt_fill_device_info(struct dt_node_info *info, int node);
int dt_get_node(struct dt_node_info *info, int offset, const char *compat);
int dt_get_stdout_uart_info(struct dt_node_info *info);
int dt_get_stdout_node_offset(void);
uint32_t dt_get_ddr_size(void);
const char *dt_get_board_model(void);

#endif /* __STM32MP1_DT_H__ */
