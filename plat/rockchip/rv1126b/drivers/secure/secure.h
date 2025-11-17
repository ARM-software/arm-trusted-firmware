/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#ifndef SECURE_H
#define SECURE_H

/* PMUSGRF */
#define PMUSGRF_SOC_CON(i)		((i) * 4)
#define PMUSGRF_PMUMCU_BOOT_ADDR	0x20

/* SYSSGRF */
#define SYSSGRF_CON			0x0
#define SYSSGRF_HPMCU_BOOT_ADDR		0xc
#define SYSSGRF_APB_SECURE_CON		0x10
#define SYSSGRF_AHB_SECURE_CON		0x14
#define SYSSGRF_AXI_SECURE_CON		0x18

#define SYSSGRF_FIREWALL_CON(i)		(0x20 + (i) * 4)
#define SYSSGRF_FIREWALL_CON_CNT	12
#define SYSSGRF_FUSE_CON		0x50

/* FW_DDR */
#define FIREWALL_DDR_RGN(i)		((i) * 0x4)
#define FIREWALL_DDR_RGN_CNT		8
#define FIREWALL_DDR_MST(i)		(0x40 + (i) * 0x4)
#define FIREWALL_DDR_MST_CNT		16
#define FIREWALL_DDR_CON		0xf0

#define PLAT_MAX_DDR_CAPACITY_MB	4096
#define RG_MAP_SECURE(top, base)	((((top) - 1) << 16) | ((base) & 0xffff))
#define MB_TO_128KB_SHFT		3

#define FIREWALL_SYSMEM_RGN_CNT		2

#define RG_MAP_SRAM_SECURE(top_kb, base_kb)	\
	(((((top_kb) / 4 - 1) & 0xf) << 4) | ((base_kb) / 4 & 0xf))

void secure_timer_init(void);
void secure_init(void);

#endif /* SECURE_H */
