/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/extensions/sys_reg_trace.h>

void sys_reg_trace_init_el3(void)
{
	uint32_t val;

	/*
	 * NSACR.NSTRCDIS = b0
	 * enable NS system register access to implemented trace
	 * registers.
	 */
	val = read_nsacr();
	val &= ~NSTRCDIS_BIT;
	write_nsacr(val);
}
