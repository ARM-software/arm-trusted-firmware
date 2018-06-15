/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <common_def.h>

#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#define PLATFORM_STACK_SIZE		0x400

#define BL31_BASE			0x04000000
#define BL31_SIZE			0x00080000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define SQ_BOOT_CFG_ADDR			0x45410000
#define PLAT_SQ_PRIMARY_CPU_SHIFT		8
#define PLAT_SQ_PRIMARY_CPU_BIT_WIDTH		6

#endif /* __PLATFORM_DEF_H__ */
