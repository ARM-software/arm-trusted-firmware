/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <stdint.h>

#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <drivers/cadence/cdns_uart.h>

void zynqmp_config_setup(void);

uint32_t zynqmp_calc_core_pos(u_register_t mpidr);

/* ZynqMP specific functions */
uint32_t zynqmp_get_uart_clk(void);
uint32_t zynqmp_get_bootmode(void);


#if ZYNQMP_WDT_RESTART
/*
 * Register handler to specific GIC entrance
 * for INTR_TYPE_EL3 type of interrupt
 */
int request_intr_type_el3(uint32_t, interrupt_type_handler_t);
#endif

#endif /* PLAT_PRIVATE_H */
