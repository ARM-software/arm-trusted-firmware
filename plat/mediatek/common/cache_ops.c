/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/mmio.h>

#include <cache_ops.h>
#include <mcucfg.h>

#define L3_SHARE_EN	9
#define L3_SHARE_PRE_EN	8

void disable_cache_as_ram(void)
{
	unsigned long v;

	mmio_clrbits_32(MP0_CLUSTER_CFG0, 1 << L3_SHARE_EN);
	dsb();

	__asm__ volatile ("mrs %0, S3_0_C15_C3_5" : "=r" (v));
	v |= (0xf << 4);
	__asm__ volatile ("msr S3_0_C15_C3_5, %0" : : "r" (v));
	dsb();

	do {
		__asm__ volatile ("mrs %0, S3_0_C15_C3_7" : "=r" (v));
	} while (((v >> 0x4) & 0xf) != 0xf);

	mmio_clrbits_32(MP0_CLUSTER_CFG0, 1 << L3_SHARE_PRE_EN);
	dsb();
}
