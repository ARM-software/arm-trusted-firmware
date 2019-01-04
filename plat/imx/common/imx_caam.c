/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/mmio.h>

#include <imx_caam.h>

void imx_caam_init(void)
{
	struct caam_ctrl *caam = (struct caam_ctrl *)CAAM_AIPS_BASE;
	uint32_t reg;
	int i;

	for (i = 0; i < CAAM_NUM_JOB_RINGS; i++) {
		reg = mmio_read_32((uintptr_t)&caam->jr[i].jrmidr_ms);
		reg |= JROWN_NS | JROWN_MID;
		mmio_write_32((uintptr_t)&caam->jr[i].jrmidr_ms, reg);
	}
}
