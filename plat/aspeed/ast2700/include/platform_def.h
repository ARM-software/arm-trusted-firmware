/*
 * Copyright (c) 2023, Aspeed Technology Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>
#include <platform_reg.h>

#define PLATFORM_STACK_SIZE		UL(0x1000)

/* cpu topology */
#define PLATFORM_SYSTEM_COUNT		U(1)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CORE_PRIMARY		U(0)
#define PLATFORM_CORE_COUNT_PER_CLUSTER U(4)
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_CORE_COUNT_PER_CLUSTER)

/* arch timer */
#define PLAT_SYSCNT_CLKIN_HZ		U(1600000000)

/* power domain */
#define PLAT_MAX_PWR_LVL		U(1)
#define PLAT_NUM_PWR_DOMAINS		U(5)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

/* cache line size */
#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

/* translation tables */
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 40)
#define MAX_XLAT_TABLES			U(8)
#define MAX_MMAP_REGIONS		U(32)

/* BL31 region */
#define BL31_BASE			ULL(0x430000000)
#define BL31_SIZE			SZ_512K
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

/* BL32 region */
#define BL32_BASE			BL31_LIMIT
#define BL32_SIZE			SZ_16M
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)

/* console */
#define CONSOLE_UART_BASE		UART12_BASE
#define CONSOLE_UART_CLKIN_HZ		U(1846153)
#define CONSOLE_UART_BAUDRATE		U(115200)

#endif /* PLATFORM_DEF_H */
