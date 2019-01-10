/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <stdint.h>

#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>

void zynqmp_config_setup(void);

unsigned int zynqmp_calc_core_pos(u_register_t mpidr);

/* ZynqMP specific functions */
unsigned int zynqmp_get_uart_clk(void);
unsigned int zynqmp_get_bootmode(void);

/* For FSBL handover */
enum fsbl_handoff {
	FSBL_HANDOFF_SUCCESS = 0,
	FSBL_HANDOFF_NO_STRUCT,
	FSBL_HANDOFF_INVAL_STRUCT,
	FSBL_HANDOFF_TOO_MANY_PARTS,
};

#if ZYNQMP_WDT_RESTART
/*
 * Register handler to specific GIC entrance
 * for INTR_TYPE_EL3 type of interrupt
 */
int request_intr_type_el3(uint32_t, interrupt_type_handler_t);
#endif

enum fsbl_handoff fsbl_atf_handover(entry_point_info_t *bl32_image_ep_info,
		       entry_point_info_t *bl33_image_ep_info);

#endif /* PLAT_PRIVATE_H */
