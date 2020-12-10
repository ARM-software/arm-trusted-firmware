/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved. \
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mtk_ptp3_common.h"

/************************************************
 * Central control: turn on sysPi protection
 ************************************************/
static unsigned int ptp3_cfg1[NR_PTP3_CFG1_CPU][NR_PTP3_CFG] = {
	{0x0C530610, 0x110842},
	{0x0C530E10, 0x110842},
	{0x0C531610, 0x110842},
	{0x0C531E10, 0x110842},
	{0x0C532610, 0x110842},
	{0x0C532E10, 0x110842},
	{0x0C533610, 0x110842},
	{0x0C533E10, 0x110842}
};
static unsigned int ptp3_cfg2[NR_PTP3_CFG2_CPU][NR_PTP3_CFG] = {
	{0x0C53B830, 0x68000},
	{0x0C53BA30, 0x68000},
	{0x0C53BC30, 0x68000},
	{0x0C53BE30, 0x68000}
};
static unsigned int ptp3_cfg3[NR_PTP3_CFG3_CPU][NR_PTP3_CFG] = {
	{0x0C532480, 0x7C607C6},
	{0x0C532C80, 0x7C607C6},
	{0x0C533480, 0x7C607C6},
	{0x0C533C80, 0x7C607C6}
};

/************************************************
 * API
 ************************************************/
void ptp3_init(unsigned int core)
{
	unsigned int _core;

	/* Apply ptp3_cfg1 for core 0 to 7 */
	if (core < NR_PTP3_CFG1_CPU) {
		/* update ptp3_cfg1 */
		ptp3_write(
			ptp3_cfg1[core][PTP3_CFG_ADDR],
			ptp3_cfg1[core][PTP3_CFG_VALUE]);
	}

	/* Apply ptp3_cfg2 for core 4 to 7 */
	if (core >= PTP3_CFG2_CPU_START_ID) {
		_core = core - PTP3_CFG2_CPU_START_ID;

		if (_core < NR_PTP3_CFG2_CPU) {
			/* update ptp3_cfg2 */
			ptp3_write(
				ptp3_cfg2[_core][PTP3_CFG_ADDR],
				ptp3_cfg2[_core][PTP3_CFG_VALUE]);
		}
	}

	/* Apply ptp3_cfg3 for core 4 to 7 */
	if (core >= PTP3_CFG3_CPU_START_ID) {
		_core = core - PTP3_CFG3_CPU_START_ID;

		if (_core < NR_PTP3_CFG3_CPU) {
			/* update ptp3_cfg3 */
			ptp3_write(
				ptp3_cfg3[_core][PTP3_CFG_ADDR],
				ptp3_cfg3[_core][PTP3_CFG_VALUE]);
		}
	}
}

void ptp3_deinit(unsigned int core)
{
	if (core < NR_PTP3_CFG1_CPU) {
		/* update ptp3_cfg1 */
		ptp3_write(
			ptp3_cfg1[core][PTP3_CFG_ADDR],
			ptp3_cfg1[core][PTP3_CFG_VALUE] &
				 ~PTP3_CFG1_MASK);
	}
}
