/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <m0_param.h>
#include "rk3399_mcu.h"

__attribute__((noreturn)) void main(void)
{
	switch (mmio_read_32(PARAM_ADDR + PARAM_M0_FUNC)) {
	case M0_FUNC_SUSPEND:
		handle_suspend();
		break;
	case M0_FUNC_DRAM:
		handle_dram();
		break;
	default:
		break;
	}

	mmio_write_32(PARAM_ADDR + PARAM_M0_DONE, M0_DONE_FLAG);

	for (;;)
		__asm__ volatile ("wfi");
}
