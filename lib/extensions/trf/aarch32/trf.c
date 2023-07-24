/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/extensions/trf.h>

void trf_init_el3(void)
{
	uint32_t val;

	/*
	 * Allow access of trace filter control registers from
	 * non-monitor mode.
	 */
	val = read_sdcr();
	val &= ~(SDCR_TTRF_BIT);
	write_sdcr(val);
}
