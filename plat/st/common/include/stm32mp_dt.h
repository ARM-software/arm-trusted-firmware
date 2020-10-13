/*
 * Copyright (c) 2020, STMicroelectronics - All Rights Reserved
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
int dt_set_stdout_pinctrl(void);
void dt_fill_device_info(struct dt_node_info *info, int node);
int dt_get_node(struct dt_node_info *info, int offset, const char *compat);
int dt_get_stdout_uart_info(struct dt_node_info *info);
uint32_t dt_get_ddr_size(void);
uint32_t dt_get_pwr_vdd_voltage(void);
const char *dt_get_board_model(void);
int fdt_get_gpio_bank_pin_count(unsigned int bank);

#endif /* STM32MP_DT_H */
