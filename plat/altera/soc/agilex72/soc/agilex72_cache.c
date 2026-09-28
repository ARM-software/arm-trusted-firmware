/*
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>

#include "agilex72_cache.h"

void invalidate_dcache_all(void)
{
	dcsw_op_all(DCISW);
}

void invalidate_l1_i_cache(void)
{
	__asm__ volatile("ic iallu");
	dsb();
	isb();
}

void invalidate_tlb_low_el(void)
{
	u_register_t scr = read_scr_el3();

	write_scr_el3(scr | SCR_NS_BIT);
	isb();
	tlbialle2();
	dsb();
	tlbialle1();
	dsb();
	write_scr_el3(scr);
	isb();
}

void invalidate_cache_low_el(void)
{
	invalidate_tlb_low_el();
}

void invalidate_all_tlbs(void)
{
	tlbialle3();
	dsb();
	invalidate_tlb_low_el();
}
