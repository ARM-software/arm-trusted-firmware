/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx_regs.h>
#include <imx_snvs.h>

void imx_snvs_init(void)
{
	struct snvs *snvs = (struct snvs *)SNVS_BASE;
	uintptr_t addr;
	uint32_t val;

	addr = (uintptr_t)&snvs->hpcomr;
	val = mmio_read_32(addr);
	val |= HPCOMR_NPSWA_EN;
	mmio_write_32(addr, val);
}
