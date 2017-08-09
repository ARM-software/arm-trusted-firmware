/*
 * Copyright (c) 2014-2016, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <mmio.h>
#include <pl310.h>

void pl310_enable(uintptr_t pl310_base)
{
	mmio_setbits_32(pl310_base + PL310_CTRL, PL310_CTRL_ENABLE_BIT);

	/* if PL310 AUX_CTRL[FLZW] is enable, enable ACTLR[FLZW] */
	if (mmio_read_32(pl310_base + PL310_AUX_CTRL) & PL310_AUX_FLZ_BIT)
		write_actlr(read_actlr() | (1 << 3));	// FIXME: macro for FLZW bit
}

void pl310_disable(uintptr_t pl310_base)
{
	/* if PL310 AUX_CTRL[FLZW] is disable, disable ACTLR[FLZW] */
	if (mmio_read_32(pl310_base + PL310_AUX_CTRL) & PL310_AUX_FLZ_BIT)
		write_actlr(read_actlr() & ~(1 << 3));	// FIXME: macro for FLZW bit

	mmio_clrbits_32(pl310_base + PL310_CTRL, PL310_CTRL_ENABLE_BIT);
}

void pl310_lock_all_ways(uintptr_t pl310_base)
{
	uintptr_t pl310_lock = pl310_base + PL310_DCACHE_LOCKDOWN_BASE;
	uint32_t mask = PL310_8WAYS_MASK;
	size_t n = PL310_LOCKDOWN_NBREGS;

	if (mmio_read_32(pl310_base + PL310_AUX_CTRL) & PL310_AUX_16WAY_BIT)
		mask |= PL310_16WAYS_UPPERMASK;

	while (n--) {
		/* lock data cache */
		mmio_write_32(pl310_lock, mask);
		pl310_lock += PL310_LOCKDOWN_SZREG;
		/* lock instruction cache */
		mmio_write_32(pl310_lock, mask);
		pl310_lock += PL310_LOCKDOWN_SZREG;
	}
}
