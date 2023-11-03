/*
 * Copyright (c) 2014-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <stdint.h>

#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <drivers/cadence/cdns_uart.h>
#include <lib/xlat_tables/xlat_tables.h>

void zynqmp_config_setup(void);

const mmap_region_t *plat_get_mmap(void);

uint32_t zynqmp_calc_core_pos(u_register_t mpidr);

/* ZynqMP specific functions */
uint32_t get_uart_clk(void);
uint32_t zynqmp_get_bootmode(void);

#if ZYNQMP_WDT_RESTART
typedef struct zynqmp_intr_info_type_el3 {
	uint32_t id;
	interrupt_type_handler_t handler;
} zynmp_intr_info_type_el3_t;

/*
 * Register handler to specific GIC entrance
 * for INTR_TYPE_EL3 type of interrupt
 */
int request_intr_type_el3(uint32_t, interrupt_type_handler_t);
#endif

#endif /* PLAT_PRIVATE_H */
