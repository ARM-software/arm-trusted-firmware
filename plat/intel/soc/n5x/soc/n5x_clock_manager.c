/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "n5x_clock_manager.h"
#include "n5x_system_manager.h"
#include "socfpga_handoff.h"

uint64_t clk_get_pll_output_hz(void)
{
	uint32_t clksrc;
	uint32_t scr_reg;
	uint32_t divf;
	uint32_t divr;
	uint32_t divq;
	uint32_t power = 1;
	uint64_t clock = 0;

	clksrc = ((get_clk_freq(CLKMGR_PERPLL_PLLGLOB)) &
			CLKMGR_PLLGLOB_VCO_PSRC_MASK) >> CLKMGR_PLLGLOB_VCO_PSRC_OFFSET;

	switch (clksrc) {
	case CLKMGR_VCO_PSRC_EOSC1:
		scr_reg = SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1);
		clock = mmio_read_32(scr_reg);
		break;

	case CLKMGR_VCO_PSRC_INTOSC:
		clock = CLKMGR_INTOSC_HZ;
		break;

	case CLKMGR_VCO_PSRC_F2S:
		scr_reg = SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_2);
		clock = mmio_read_32(scr_reg);
		break;
	}

	divf = ((get_clk_freq(CLKMGR_PERPLL_PLLDIV)) &
			CLKMGR_PLLDIV_FDIV_MASK) >> CLKMGR_PLLDIV_FDIV_OFFSET;
	divr = ((get_clk_freq(CLKMGR_PERPLL_PLLDIV)) &
			CLKMGR_PLLDIV_REFCLKDIV_MASK) >> CLKMGR_PLLDIV_REFCLKDIV_OFFSET;
	divq = ((get_clk_freq(CLKMGR_PERPLL_PLLDIV)) &
			CLKMGR_PLLDIV_OUTDIV_QDIV_MASK) >> CLKMGR_PLLDIV_OUTDIV_QDIV_OFFSET;

	while (divq) {
		power *= 2;
		divq--;
	}

	return ((clock * 2 * (divf + 1)) / ((divr + 1) * power));
}

uint64_t get_l4_clk(void)
{
	uint32_t clock = 0;
	uint32_t mainpll_c1cnt;
	uint32_t perpll_c1cnt;
	uint32_t clksrc;

	mainpll_c1cnt = ((get_clk_freq(CLKMGR_MAINPLL_PLLOUTDIV)) &
			CLKMGR_PLLOUTDIV_C1CNT_MASK) >> CLKMGR_PLLOUTDIV_C1CNT_OFFSET;

	perpll_c1cnt = ((get_clk_freq(CLKMGR_PERPLL_PLLOUTDIV)) &
			CLKMGR_PLLOUTDIV_C1CNT_MASK) >> CLKMGR_PLLOUTDIV_C1CNT_OFFSET;

	clksrc = ((get_clk_freq(CLKMGR_MAINPLL_NOCCLK)) & CLKMGR_CLKSRC_MASK) >>
			CLKMGR_CLKSRC_OFFSET;

	switch (clksrc) {
	case CLKMGR_CLKSRC_MAIN:
		clock = clk_get_pll_output_hz();
		clock /= 1 + mainpll_c1cnt;
		break;

	case CLKMGR_CLKSRC_PER:
		clock = clk_get_pll_output_hz();
		clock /= 1 + perpll_c1cnt;
		break;

	default:
		return 0;
		break;
	}

	clock /= BIT(((get_clk_freq(CLKMGR_MAINPLL_NOCDIV)) >>
			CLKMGR_NOCDIV_L4MAIN_OFFSET) & CLKMGR_NOCDIV_DIVIDER_MASK);

	return clock;
}

/* Return MPU clock */
uint32_t get_mpu_clk(void)
{
	uint32_t clock = 0;
	uint32_t mainpll_c0cnt;
	uint32_t perpll_c0cnt;
	uint32_t clksrc;

	mainpll_c0cnt = ((get_clk_freq(CLKMGR_MAINPLL_PLLOUTDIV)) &
			CLKMGR_PLLOUTDIV_C0CNT_MASK) >> CLKMGR_PLLOUTDIV_C0CNT_OFFSET;

	perpll_c0cnt = ((get_clk_freq(CLKMGR_PERPLL_PLLOUTDIV)) &
			CLKMGR_PLLOUTDIV_C0CNT_MASK) >> CLKMGR_PLLOUTDIV_C0CNT_OFFSET;

	clksrc = ((get_clk_freq(CLKMGR_MAINPLL_NOCCLK)) & CLKMGR_CLKSRC_MASK) >>
			CLKMGR_CLKSRC_OFFSET;

	switch (clksrc) {
	case CLKMGR_CLKSRC_MAIN:
		clock = clk_get_pll_output_hz();
		clock /= 1 + mainpll_c0cnt;
		break;

	case CLKMGR_CLKSRC_PER:
		clock = clk_get_pll_output_hz();
		clock /= 1 + perpll_c0cnt;
		break;

	default:
		return 0;
		break;
	}

	clock /= BIT(((get_clk_freq(CLKMGR_MAINPLL_NOCDIV)) >>
			CLKMGR_NOCDIV_L4MAIN_OFFSET) & CLKMGR_NOCDIV_DIVIDER_MASK);

	return clock;
}

/* Calculate clock frequency based on parameter */
uint32_t get_clk_freq(uint32_t psrc_reg)
{
	uint32_t clk_psrc;

	clk_psrc = mmio_read_32(CLKMGR_N5X_BASE + psrc_reg);

	return clk_psrc;
}

/* Get cpu freq clock */
uint32_t get_cpu_clk(void)
{
	uint32_t cpu_clk = 0;

	cpu_clk = get_mpu_clk()/PLAT_HZ_CONVERT_TO_MHZ;

	return cpu_clk;
}

/* Return mpu_periph_clk clock frequency */
uint32_t get_mpu_periph_clk(void)
{
	uint32_t mpu_periph_clk = 0;
	/* mpu_periph_clk is mpu_clk, via a static /4 divider  */
	mpu_periph_clk = (get_mpu_clk()/4)/PLAT_HZ_CONVERT_TO_MHZ;
	return mpu_periph_clk;
}

/* Return mpu_periph_clk tick */
unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_SYS_COUNTER_FREQ_IN_TICKS;
}
