/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ZYNQMP_PRIVATE_H__
#define __ZYNQMP_PRIVATE_H__

#include <interrupt_mgmt.h>

void zynqmp_config_setup(void);

/* ZynqMP specific functions */
unsigned int zynqmp_get_uart_clk(void);
int zynqmp_is_pmu_up(void);
unsigned int zynqmp_get_bootmode(void);

/* For FSBL handover */
void fsbl_atf_handover(entry_point_info_t *bl32_image_ep_info,
		       entry_point_info_t *bl33_image_ep_info);

#endif /* __ZYNQMP_PRIVATE_H__ */
