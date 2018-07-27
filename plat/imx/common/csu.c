/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mmio.h>
#include <imx-regs.h>
#include <csu.h>

void csu_init(void)
{
	int i;
	uintptr_t *csl_reg = (uintptr_t *)CSU_BASE;

	for (i = 0; i < MXC_MAX_CSU_REGS; i++, csl_reg++)
		mmio_write_32((uintptr_t)csl_reg, CSU_CSL_OPEN_ACCESS);
}
