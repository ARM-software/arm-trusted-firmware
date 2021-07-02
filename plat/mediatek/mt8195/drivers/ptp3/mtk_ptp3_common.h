/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_PTP3_COMMON_H
#define MTK_PTP3_COMMON_H

#include <lib/mmio.h>
#include <lib/utils_def.h>

/************************************************
 * CPU info
 ************************************************/
#define NR_PTP3_CFG_CPU			U(8)
#define PTP3_CFG_CPU_START_ID_L		U(0)
#define PTP3_CFG_CPU_START_ID_B		U(4)
#define PTP3_CFG_CPU_END_ID		U(7)

#define NR_PTP3_CFG1_DATA		U(2)
#define PTP3_CFG1_MASK			0x3000

#define NR_PTP3_CFG2_DATA		U(5)

#define PTP3_CFG3_MASK1			0x1180
#define PTP3_CFG3_MASK2			0x35C0
#define PTP3_CFG3_MASK3			0x3DC0

/************************************************
 * register read/write
 ************************************************/
#define ptp3_write(addr, val) mmio_write_32((uintptr_t)addr, val)
#define ptp3_clrsetbits(addr, clear, set) \
	mmio_clrsetbits_32((uintptr_t)addr, clear, set)

/************************************************
 * config enum
 ************************************************/
enum PTP3_CFG {
	PTP3_CFG_ADDR,
	PTP3_CFG_VALUE,
	NR_PTP3_CFG,
};

/************************************
 * prototype
 ************************************/
extern void ptp3_core_init(unsigned int core);
extern void ptp3_core_unInit(unsigned int core);

#endif /* MTK_PTP3_COMMON_H */
