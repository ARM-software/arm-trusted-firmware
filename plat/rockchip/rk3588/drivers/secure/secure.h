/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_H
#define SECURE_H

/* DSUSGRF */
#define DSU_SGRF_SOC_CON(i)		((i) * 4)
#define DSUSGRF_SOC_CON(i)		((i) * 4)
#define DSUSGRF_SOC_CON_CNT		13
#define DSUSGRF_DDR_HASH_CON(i)		(0x240 + (i) * 4)
#define DSUSGRF_DDR_HASH_CON_CNT	8

/* PMUSGRF */
#define PMU1SGRF_SOC_CON(n)		((n) * 4)

/* SGRF */
#define SGRF_SOC_CON(i)			((i) * 4)
#define SGRF_FIREWALL_CON(i)		(0x240 + (i) * 4)
#define SGRF_FIREWALL_CON_CNT		32

/* ddr firewall */
#define FIREWALL_DDR_RGN(i)		((i) * 0x4)
#define FIREWALL_DDR_RGN_CNT		16
#define FIREWALL_DDR_MST(i)		(0x40 + (i) * 0x4)
#define FIREWALL_DDR_MST_CNT		42
#define FIREWALL_DDR_CON		0xf0

#define FIREWALL_SYSMEM_RGN(i)		((i) * 0x4)
#define FIREWALL_SYSMEM_RGN_CNT		8
#define FIREWALL_SYSMEM_MST(i)		(0x40 + (i) * 0x4)
#define FIREWALL_SYSMEM_MST_CNT		43
#define FIREWALL_SYSMEM_CON		0xf0

#define FIREWALL_DSU_RGN(i)		((i) * 0x4)
#define FIREWALL_DSU_RGN_CNT		16
#define FIREWALL_DSU_MST(i)		(0x40 + (i) * 0x4)
#define FIREWALL_DSU_MST_CNT		2
#define FIREWALL_DSU_CON(i)		(0xf0 + (i) * 4)
#define FIREWALL_DSU_CON_CNT		4

#define PLAT_MAX_DDR_CAPACITY_MB	0x8000	/* for 32Gb */
#define RG_MAP_SECURE(top, base)	\
	(((((top) - 1) & 0x7fff) << 16) | ((base) & 0x7fff))
#define RG_MAP_SRAM_SECURE(top_kb, base_kb)	\
	(((((top_kb) / 4 - 1) & 0xff) << 16) | ((base_kb) / 4 & 0xff))

void secure_timer_init(void);
void sgrf_init(void);

#endif /* SECURE_H */
