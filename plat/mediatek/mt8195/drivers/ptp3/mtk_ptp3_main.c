/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved. \
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <mtk_ptp3_common.h>

#define PTP3_CORE_OFT(core)	(0x800 * (core))

/************************************************
 * Central control
 ************************************************/
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

static void ptp3_init(unsigned int core)
{
	unsigned int i, addr, value;

	if (core < PTP3_CFG_CPU_START_ID_B) {
		ptp3_clrsetbits(ptp3_cfg1[0][PTP3_CFG_ADDR], PTP3_CFG1_MASK,
				ptp3_cfg1[0][PTP3_CFG_VALUE]);
	} else {
		ptp3_clrsetbits(ptp3_cfg1[1][PTP3_CFG_ADDR], PTP3_CFG1_MASK,
				ptp3_cfg1[1][PTP3_CFG_VALUE]);
	}

	if (core < PTP3_CFG_CPU_START_ID_B) {
		for (i = 0; i < NR_PTP3_CFG2_DATA; i++) {
			addr = ptp3_cfg2[i][PTP3_CFG_ADDR] +
			       PTP3_CORE_OFT(core);
			value = ptp3_cfg2[i][PTP3_CFG_VALUE];

			ptp3_write(addr, value);
		}
	} else {
		for (i = 0; i < NR_PTP3_CFG2_DATA; i++) {
			addr = ptp3_cfg2[i][PTP3_CFG_ADDR] +
			       PTP3_CORE_OFT(core);

			if (i == 2) {
				value = ptp3_cfg2[i][PTP3_CFG_VALUE] + 0x5E0;
			} else {
				value = ptp3_cfg2[i][PTP3_CFG_VALUE];
			}
			ptp3_write(addr, value);
		}
	}

	if (core < PTP3_CFG_CPU_START_ID_B) {
		addr = ptp3_cfg3[PTP3_CFG_ADDR] + PTP3_CORE_OFT(core);
		value = ptp3_cfg3[PTP3_CFG_VALUE];

		ptp3_write(addr, value & PTP3_CFG3_MASK1);
		ptp3_write(addr, value & PTP3_CFG3_MASK2);
		ptp3_write(addr, value & PTP3_CFG3_MASK3);
	} else {
		addr = ptp3_cfg3_ext[PTP3_CFG_ADDR] + PTP3_CORE_OFT(core);
		value = ptp3_cfg3_ext[PTP3_CFG_VALUE];

		ptp3_write(addr, value & PTP3_CFG3_MASK1);
		ptp3_write(addr, value & PTP3_CFG3_MASK2);
		ptp3_write(addr, value & PTP3_CFG3_MASK3);
	}
}

void pdp_proc_ARM_write(unsigned int pdp_n)
{
	unsigned long v = 0;

	dsb();
	__asm__ volatile ("mrs %0, S3_6_C15_C2_0" : "=r" (v));
	v |= (UL(0x0) << 52);
	v |= (UL(0x1) << 53);
	v |= (UL(0x0) << 54);
	v |= (UL(0x0) << 48);
	v |= (UL(0x1) << 49);
	__asm__ volatile ("msr S3_6_C15_C2_0, %0" : : "r" (v));
	dsb();
}

void pdp_init(unsigned int pdp_cpu, unsigned int en)
{
	if ((pdp_cpu >= PTP3_CFG_CPU_START_ID_B) &&
	    (pdp_cpu < NR_PTP3_CFG_CPU)) {
		pdp_proc_ARM_write(pdp_cpu);
	}
}

static void dt_proc_ARM_write(unsigned int dt_n)
{
	unsigned long v = 0;

	dsb();
	__asm__ volatile ("mrs %0, S3_6_C15_C2_0" : "=r" (v));
	v |= (UL(0x0) << 33);
	v |= (UL(0x0) << 32);
	__asm__ volatile ("msr S3_6_C15_C2_0, %0" : : "r" (v));
	dsb();
}

void dt_init(unsigned int dt_cpu, unsigned int en)
{
	if ((dt_cpu >= PTP3_CFG_CPU_START_ID_B) &&
	    (dt_cpu < NR_PTP3_CFG_CPU)) {
		dt_proc_ARM_write(dt_cpu);
	}
}
void ptp3_core_init(unsigned int core)
{
	/* init for ptp3 */
	ptp3_init(core);
	/* init for pdp */
	pdp_init(core, 1);
	/* init for dt */
	dt_init(core, 1);
}

void ptp3_core_unInit(unsigned int core)
{
	/* TBD */
}
