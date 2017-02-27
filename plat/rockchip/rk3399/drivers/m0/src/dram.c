/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

void handle_dram(void)
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
}
