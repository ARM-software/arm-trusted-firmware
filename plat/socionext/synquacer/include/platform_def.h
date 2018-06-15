/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <common_def.h>

/* CPU topology */
#define PLAT_MAX_CORES_PER_CLUSTER	2
#define PLAT_CLUSTER_COUNT		12
#define PLATFORM_CORE_COUNT		(PLAT_CLUSTER_COUNT *	\
					 PLAT_MAX_CORES_PER_CLUSTER)

#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#define PLATFORM_STACK_SIZE		0x400

#define BL31_BASE			0x04000000
#define BL31_SIZE			0x00080000
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)

#define PLAT_SQ_CCN_BASE		0x32000000
#define PLAT_SQ_CLUSTER_TO_CCN_ID_MAP					\
					0,	/* Cluster 0 */		\
					18,	/* Cluster 1 */		\
					11,	/* Cluster 2 */		\
					29,	/* Cluster 3 */		\
					35,	/* Cluster 4 */		\
					17,	/* Cluster 5 */		\
					12,	/* Cluster 6 */		\
					30,	/* Cluster 7 */		\
					14,	/* Cluster 8 */		\
					32,	/* Cluster 9 */		\
					15,	/* Cluster 10 */	\
					33	/* Cluster 11 */

/* UART related constants */
#define PLAT_SQ_BOOT_UART_BASE		0x2A400000
#define PLAT_SQ_BOOT_UART_CLK_IN_HZ	62500000
#define SQ_CONSOLE_BAUDRATE		115200

#define SQ_SYS_CNTCTL_BASE		0x2a430000

#define SQ_SYS_TIMCTL_BASE		0x2a810000
#define PLAT_SQ_NSTIMER_FRAME_ID	0

#define SQ_BOOT_CFG_ADDR			0x45410000
#define PLAT_SQ_PRIMARY_CPU_SHIFT		8
#define PLAT_SQ_PRIMARY_CPU_BIT_WIDTH		6

#define PLAT_SQ_GICD_BASE		0x30000000
#define PLAT_SQ_GICR_BASE		0x30400000

#endif /* __PLATFORM_DEF_H__ */
