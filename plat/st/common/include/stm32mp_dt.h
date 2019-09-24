/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_DT_H
#define STM32MP_DT_H

#include <stdbool.h>

#define DT_DISABLED		U(0)
#define DT_NON_SECURE		U(1)
#define DT_SECURE		U(2)
#define DT_SHARED		(DT_NON_SECURE | DT_SECURE)

struct dt_node_info {
	uint32_t base;
	int32_t clock;
	int32_t reset;
	uint32_t status;
};

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
int dt_open_and_check(void);
int fdt_get_address(void **fdt_addr);
bool fdt_check_node(int node);
uint8_t fdt_get_status(int node);
uint32_t fdt_read_uint32_default(int node, const char *prop_name,
				 uint32_t dflt_value);
int fdt_read_uint32_array(int node, const char *prop_name,
			  uint32_t *array, uint32_t count);
int fdt_get_reg_props_by_name(int node, const char *name, uintptr_t *base,
			      size_t *size);
int dt_set_stdout_pinctrl(void);
void dt_fill_device_info(struct dt_node_info *info, int node);
int dt_get_node(struct dt_node_info *info, int offset, const char *compat);
int dt_get_stdout_uart_info(struct dt_node_info *info);
uint32_t dt_get_ddr_size(void);
uintptr_t dt_get_ddrctrl_base(void);
uintptr_t dt_get_ddrphyc_base(void);
uintptr_t dt_get_pwr_base(void);
uint32_t dt_get_pwr_vdd_voltage(void);
uintptr_t dt_get_syscfg_base(void);
const char *dt_get_board_model(void);

#endif /* STM32MP_DT_H */
