/*
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define CPG_BASE	0xE6150000U
#define CPG_CPGWPR	(CPG_BASE + 0x000U)

#define MSSR_BASE	CPG_BASE
#define MSSR_SRCR(n)	(MSSR_BASE + 0x2C00U + (n) * 4)
#define MSSR_SRSTCLR(n)	(MSSR_BASE + 0x2C80U + (n) * 4)
#define MSSR_MSTPCR(n)	(MSSR_BASE + 0x2D00U + (n) * 4)
#define MSSR_MSTPSR(n)	(MSSR_BASE + 0x2E00U + (n) * 4)

static void cpg_write_32(uint32_t addr, uint32_t val)
{
	mmio_write_32(CPG_CPGWPR, ~val);
	mmio_write_32(addr, val);
}

void rcar_mssr_clock(unsigned int n, uint32_t data, bool on, bool force)
{
	uint32_t prev_status, next_status;

	prev_status = mmio_read_32(MSSR_MSTPSR(n));

	if (on)
		next_status = prev_status & ~data;
	else
		next_status = prev_status | data;

	if (!force && (prev_status == next_status))
		return;

	cpg_write_32(MSSR_MSTPCR(n), next_status);

	if (on)
		while ((data & mmio_read_32(MSSR_MSTPSR(n))) != 0)
			;
	else
		while ((data & mmio_read_32(MSSR_MSTPSR(n))) == 0)
			;
}

void rcar_mssr_soft_reset(unsigned int n, uint32_t data, bool assert, bool force)
{
	uint32_t prev_status, next_status;

	prev_status = mmio_read_32(MSSR_SRCR(n));

	if (assert)
		next_status = prev_status | data;
	else
		next_status = prev_status & ~data;

	if (!force && (prev_status == next_status))
		return;

	if (assert)
		cpg_write_32(MSSR_SRCR(n), data);
	else
		cpg_write_32(MSSR_SRSTCLR(n), data);
}

void rcar_mssr_setup(void)
{
	/* INTC-AP de-assert */
	rcar_mssr_soft_reset(5,  BIT(31), 0, 0);
	rcar_mssr_soft_reset(11, BIT(19), 0, 0);

	/* INTC-AP clock on */
	rcar_mssr_clock(5, BIT(31), 1, 0);
}
