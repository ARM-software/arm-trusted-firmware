/*
 * Copyright (c) 2021-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PTP3_PLAT_H
#define PTP3_PLAT_H

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <ptp3_common.h>

/* CPU info */
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

/* Central control */
static unsigned int ptp3_cfg1[NR_PTP3_CFG1_DATA][NR_PTP3_CFG] = {
	{0x0C53A2A0, 0x1000},
	{0x0C53A2A4, 0x1000}
};

static unsigned int ptp3_cfg2[NR_PTP3_CFG2_DATA][NR_PTP3_CFG] = {
	{0x0C530404, 0x3A1000},
	{0x0C530428, 0x13E0408},
	{0x0C530434, 0xB22800},
	{0x0C53043C, 0x750},
	{0x0C530440, 0x0222c4cc}
};

static unsigned int ptp3_cfg3[NR_PTP3_CFG] = {0x0C530400, 0x2D80};
static unsigned int ptp3_cfg3_ext[NR_PTP3_CFG] = {0x0C530400, 0xC00};

#endif /* PTP3_PLAT_H */
