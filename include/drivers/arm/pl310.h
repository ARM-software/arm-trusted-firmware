/*
 * Copyright (c) 2014-2016, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PL310_H__
#define __PL310_H__

/* PL310 registers */

#define PL310_CTRL			0x100
#define PL310_AUX_CTRL			0x104
#define PL310_TAG_RAM_CTRL		0x108
#define PL310_DATA_RAM_CTRL		0x10C

#define PL310_SYNC			0x730
#define PL310_INV_BY_WAY		0x77C
#define PL310_CLEAN_BY_WAY		0x7BC
#define PL310_FLUSH_BY_WAY		0x7FC
#define PL310_INV_BY_PA			0x770
#define PL310_CLEAN_BY_PA		0x7B0
#define PL310_FLUSH_BY_PA		0x7F0
#define PL310_FLUSH_BY_INDEXWAY		0x7F8

#define PL310_DCACHE_LOCKDOWN_BASE	0x900
#define PL310_ICACHE_LOCKDOWN_BASE	0x904

#define PL310_ADDR_FILT_START		0xC00
#define PL310_ADDR_FILT_END		0xC04

#define PL310_DEBUG_CTRL		0xF40
#define PL310_PREFETCH_CTRL		0xF60
#define PL310_POWER_CTRL		0xF80

/* registers bit fields */

#define PL310_CTRL_ENABLE_BIT		(1 << 0)

#define PL310_AUX_FLZ_BIT		(1 << 0)
#define PL310_AUX_16WAY_BIT		(1 << 16)

#define PL310_SYNC_SYNCHRO_BIT		(1 << 0)

/* misc */

#define PL310_LINE_SIZE			32
#define PL310_8_WAYS			8

#define PL310_8WAYS_MASK		0x00FF
#define PL310_16WAYS_UPPERMASK		0xFF00

#define PL310_LOCKDOWN_NBREGS		8
#define PL310_LOCKDOWN_SZREG		4

#ifndef __ASSEMBLY__

#include <cdefs.h>
#include <stdint.h>

void pl310_enable(uintptr_t pl310_base);
void pl310_disable(uintptr_t pl310_base);
void pl310_invalidate_by_way(uintptr_t pl310_base);
void pl310_clean_by_way(uintptr_t pl310_base);
void pl310_clean_invalidate_by_way(uintptr_t pl310_base);
void pl310_lock_all_ways(uintptr_t pl310_base);

#endif /* __ASSEMBLY__ */

#endif /* __PL130_H__ */
