/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dram_regs.h>
#include <m0_param.h>
#include <pmu_bits.h>
#include <pmu_regs.h>
#include "misc_regs.h"
#include "rk3399_mcu.h"

static uint32_t gatedis_con0;

static void idle_port(void)
{
	gatedis_con0 = mmio_read_32(PMUCRU_BASE + PMU_CRU_GATEDIS_CON0);
	mmio_write_32(PMUCRU_BASE + PMU_CRU_GATEDIS_CON0, 0x3fffffff);

	mmio_setbits_32(PMU_BASE + PMU_BUS_IDLE_REQ,
			(1 << PMU_IDLE_REQ_MSCH0) | (1 << PMU_IDLE_REQ_MSCH1));
	while ((mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST) &
		((1 << PMU_IDLE_ST_MSCH1) | (1 << PMU_IDLE_ST_MSCH0))) !=
		((1 << PMU_IDLE_ST_MSCH1) | (1 << PMU_IDLE_ST_MSCH0)))
		continue;
}

static void deidle_port(void)
{
	mmio_clrbits_32(PMU_BASE + PMU_BUS_IDLE_REQ,
			(1 << PMU_IDLE_REQ_MSCH0) | (1 << PMU_IDLE_REQ_MSCH1));
	while (mmio_read_32(PMU_BASE + PMU_BUS_IDLE_ST) &
	       ((1 << PMU_IDLE_ST_MSCH1) | (1 << PMU_IDLE_ST_MSCH0)))
		continue;

	/* document is wrong, PMU_CRU_GATEDIS_CON0 do not need set MASK BIT */
	mmio_write_32(PMUCRU_BASE + PMU_CRU_GATEDIS_CON0, gatedis_con0);
}

static void ddr_set_pll(void)
{
	mmio_write_32(CRU_BASE + CRU_DPLL_CON3, PLL_MODE(PLL_SLOW_MODE));

	mmio_write_32(CRU_BASE + CRU_DPLL_CON3, PLL_POWER_DOWN(1));
	mmio_write_32(CRU_BASE + CRU_DPLL_CON0,
		      mmio_read_32(PARAM_ADDR + PARAM_DPLL_CON0));
	mmio_write_32(CRU_BASE + CRU_DPLL_CON1,
		      mmio_read_32(PARAM_ADDR + PARAM_DPLL_CON1));
	mmio_write_32(CRU_BASE + CRU_DPLL_CON3, PLL_POWER_DOWN(0));

	while ((mmio_read_32(CRU_BASE + CRU_DPLL_CON2) & (1u << 31)) == 0)
		continue;

	mmio_write_32(CRU_BASE + CRU_DPLL_CON3, PLL_MODE(PLL_NORMAL_MODE));
}

__attribute__((noreturn)) void m0_main(void)
{
	mmio_setbits_32(PHY_REG(0, 927), (1 << 22));
	mmio_setbits_32(PHY_REG(1, 927), (1 << 22));
	idle_port();

	mmio_write_32(CIC_BASE + CIC_CTRL0,
		      (((0x3 << 4) | (1 << 2) | 1) << 16) |
		      (1 << 2) | 1 |
		      mmio_read_32(PARAM_ADDR + PARAM_FREQ_SELECT));
	while ((mmio_read_32(CIC_BASE + CIC_STATUS0) & (1 << 2)) == 0)
		continue;

	ddr_set_pll();
	mmio_write_32(CIC_BASE + CIC_CTRL0, 0x20002);
	while ((mmio_read_32(CIC_BASE + CIC_STATUS0) & (1 << 0)) == 0)
		continue;

	deidle_port();
	mmio_clrbits_32(PHY_REG(0, 927), (1 << 22));
	mmio_clrbits_32(PHY_REG(1, 927), (1 << 22));

	mmio_write_32(PARAM_ADDR + PARAM_M0_DONE, M0_DONE_FLAG);

	for (;;)
		__asm__ volatile ("wfi");
}
