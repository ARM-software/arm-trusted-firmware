/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "s10_clock_manager.h"
#include "s10_handoff.h"
#include "s10_system_manager.h"


void wait_pll_lock(void)
{
	uint32_t data;

	do {
		data = mmio_read_32(ALT_CLKMGR + ALT_CLKMGR_STAT);
	} while ((ALT_CLKMGR_STAT_MAINPLLLOCKED(data) == 0) ||
			(ALT_CLKMGR_STAT_PERPLLLOCKED(data) == 0));
}

void wait_fsm(void)
{
	uint32_t data;

	do {
		data = mmio_read_32(ALT_CLKMGR + ALT_CLKMGR_STAT);
	} while (ALT_CLKMGR_STAT_BUSY(data) == ALT_CLKMGR_STAT_BUSY_E_BUSY);
}

void config_clkmgr_handoff(handoff *hoff_ptr)
{
	uint32_t m_div, refclk_div, mscnt, hscnt;

	/* Bypass all mainpllgrp's clocks */
	mmio_write_32(ALT_CLKMGR_MAINPLL +
			ALT_CLKMGR_MAINPLL_BYPASS,
			0x7);
	wait_fsm();
	/* Bypass all perpllgrp's clocks */
	mmio_write_32(ALT_CLKMGR_PERPLL +
			ALT_CLKMGR_PERPLL_BYPASS,
			0x7f);
	wait_fsm();

	/* Setup main PLL dividers */
	m_div = ALT_CLKMGR_MAINPLL_FDBCK_MDIV(hoff_ptr->main_pll_fdbck);
	refclk_div = ALT_CLKMGR_MAINPLL_PLLGLOB_REFCLKDIV(
			hoff_ptr->main_pll_pllglob);
	mscnt = 200 / ((6 + m_div) / refclk_div);
	hscnt = (m_div + 6) * mscnt / refclk_div - 9;

	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_PLLGLOB,
			hoff_ptr->main_pll_pllglob);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_FDBCK,
			hoff_ptr->main_pll_fdbck);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_VCOCALIB,
			ALT_CLKMGR_MAINPLL_VCOCALIB_HSCNT_SET(hscnt) |
			ALT_CLKMGR_MAINPLL_VCOCALIB_MSCNT_SET(mscnt));
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_PLLC0,
			hoff_ptr->main_pll_pllc0);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_PLLC1,
			hoff_ptr->main_pll_pllc1);

	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_NOCDIV,
			hoff_ptr->main_pll_nocdiv);

	/* Setup peripheral PLL dividers */
	m_div = ALT_CLKMGR_PERPLL_FDBCK_MDIV(hoff_ptr->per_pll_fdbck);
	refclk_div = ALT_CLKMGR_PERPLL_PLLGLOB_REFCLKDIV(
			hoff_ptr->per_pll_pllglob);
	mscnt = 200 / ((6 + m_div) / refclk_div);
	hscnt = (m_div + 6) * mscnt / refclk_div - 9;

	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLGLOB,
			hoff_ptr->per_pll_pllglob);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_FDBCK,
			hoff_ptr->per_pll_fdbck);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_VCOCALIB,
			ALT_CLKMGR_PERPLL_VCOCALIB_HSCNT_SET(hscnt) |
			ALT_CLKMGR_PERPLL_VCOCALIB_MSCNT_SET(mscnt));
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLC0,
			hoff_ptr->per_pll_pllc0);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLC1,
			hoff_ptr->per_pll_pllc1);

	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_GPIODIV,
			ALT_CLKMGR_PERPLL_GPIODIV_GPIODBCLK_SET(
			hoff_ptr->per_pll_gpiodiv));
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_EMACCTL,
			hoff_ptr->per_pll_emacctl);


	/* Take both PLL out of reset and power up */
	mmio_setbits_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_PLLGLOB,
			ALT_CLKMGR_MAINPLL_PLLGLOB_PD_SET_MSK |
			ALT_CLKMGR_MAINPLL_PLLGLOB_RST_SET_MSK);
	mmio_setbits_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLGLOB,
			ALT_CLKMGR_PERPLL_PLLGLOB_PD_SET_MSK |
			ALT_CLKMGR_PERPLL_PLLGLOB_RST_SET_MSK);

	wait_pll_lock();

	/* Dividers for C2 to C9 only init after PLLs are lock. */
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_MPUCLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_NOCCLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR2CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR3CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR4CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR5CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR6CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR7CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR8CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR9CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR2CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR3CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR4CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR5CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR6CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR7CLK, 0xff);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR8CLK, 0xff);

	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_MPUCLK,
			hoff_ptr->main_pll_mpuclk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_NOCCLK,
			hoff_ptr->main_pll_nocclk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR2CLK,
			hoff_ptr->main_pll_cntr2clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR3CLK,
			hoff_ptr->main_pll_cntr3clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR4CLK,
			hoff_ptr->main_pll_cntr4clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR5CLK,
			hoff_ptr->main_pll_cntr5clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR6CLK,
			hoff_ptr->main_pll_cntr6clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR7CLK,
			hoff_ptr->main_pll_cntr7clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR8CLK,
			hoff_ptr->main_pll_cntr8clk);
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR9CLK,
			hoff_ptr->main_pll_cntr9clk);

	/* Peripheral PLL Clock Source and Counters/Divider */
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR2CLK,
			hoff_ptr->per_pll_cntr2clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR3CLK,
			hoff_ptr->per_pll_cntr3clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR4CLK,
			hoff_ptr->per_pll_cntr4clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR5CLK,
			hoff_ptr->per_pll_cntr5clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR6CLK,
			hoff_ptr->per_pll_cntr6clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR7CLK,
			hoff_ptr->per_pll_cntr7clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR8CLK,
			hoff_ptr->per_pll_cntr8clk);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_CNTR9CLK,
			hoff_ptr->per_pll_cntr9clk);

	/* Take all PLLs out of bypass */
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_BYPASS, 0);
	wait_fsm();
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_BYPASS, 0);
	wait_fsm();

	/* Set safe mode/ out of boot mode */
	mmio_clrbits_32(ALT_CLKMGR + ALT_CLKMGR_CTRL,
		ALT_CLKMGR_CTRL_BOOTMODE_SET_MSK);
	wait_fsm();

	/* 10 Enable mainpllgrp's software-managed clock */
	mmio_write_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_EN,
			ALT_CLKMGR_MAINPLL_EN_RESET);
	mmio_write_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_EN,
			ALT_CLKMGR_PERPLL_EN_RESET);

	/* Clear loss lock  interrupt status register that */
	/* might be set during configuration */
	mmio_write_32(ALT_CLKMGR + ALT_CLKMGR_INTRCLR,
			ALT_CLKMGR_INTRCLR_MAINLOCKLOST_SET_MSK |
			ALT_CLKMGR_INTRCLR_PERLOCKLOST_SET_MSK);

	/* Pass clock source frequency into scratch register */
	mmio_write_32(S10_SYSMGR_CORE(SYSMGR_BOOT_SCRATCH_COLD_1),
			hoff_ptr->hps_osc_clk_h);
	mmio_write_32(S10_SYSMGR_CORE(SYSMGR_BOOT_SCRATCH_COLD_2),
			hoff_ptr->fpga_clk_hz);

}

/* Extract reference clock from platform clock source */
uint32_t get_ref_clk(uint32_t pllglob)
{
	uint32_t data32, mdiv, refclkdiv, ref_clk;
	uint32_t scr_reg;

	switch (ALT_CLKMGR_PSRC(pllglob)) {
	case ALT_CLKMGR_PLLGLOB_PSRC_EOSC1:
		scr_reg = S10_SYSMGR_CORE(SYSMGR_BOOT_SCRATCH_COLD_1);
		ref_clk = mmio_read_32(scr_reg);
		break;
	case ALT_CLKMGR_PLLGLOB_PSRC_INTOSC:
		ref_clk = ALT_CLKMGR_INTOSC_HZ;
		break;
	case ALT_CLKMGR_PLLGLOB_PSRC_F2S:
		scr_reg = S10_SYSMGR_CORE(SYSMGR_BOOT_SCRATCH_COLD_2);
		ref_clk = mmio_read_32(scr_reg);
		break;
	default:
		ref_clk = 0;
		assert(0);
		break;
	}

	refclkdiv = ALT_CLKMGR_MAINPLL_PLLGLOB_REFCLKDIV(pllglob);
	data32 = mmio_read_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_FDBCK);
	mdiv = ALT_CLKMGR_MAINPLL_FDBCK_MDIV(data32);

	ref_clk = (ref_clk / refclkdiv) * (6 + mdiv);

	return ref_clk;
}

/* Calculate L3 interconnect main clock */
uint32_t get_l3_clk(uint32_t ref_clk)
{
	uint32_t noc_base_clk, l3_clk, noc_clk, data32;
	uint32_t pllc1_reg;

	noc_clk = mmio_read_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_NOCCLK);

	switch (ALT_CLKMGR_PSRC(noc_clk)) {
	case ALT_CLKMGR_SRC_MAIN:
		pllc1_reg = ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_PLLC1;
		break;
	case ALT_CLKMGR_SRC_PER:
		pllc1_reg = ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLC1;
		break;
	default:
		pllc1_reg = 0;
		assert(0);
		break;
	}

	data32 = mmio_read_32(pllc1_reg);
	noc_base_clk = ref_clk / (data32 & 0xff);
	l3_clk = noc_base_clk / (noc_clk + 1);

	return l3_clk;
}

/* Calculate clock frequency to be used for watchdog timer */
uint32_t get_wdt_clk(void)
{
	uint32_t data32, ref_clk, l3_clk, l4_sys_clk;

	data32 = mmio_read_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_PLLGLOB);
	ref_clk = get_ref_clk(data32);

	l3_clk = get_l3_clk(ref_clk);

	l4_sys_clk = l3_clk / 4;

	return l4_sys_clk;
}

/* Calculate clock frequency to be used for UART driver */
uint32_t get_uart_clk(void)
{
	uint32_t data32, ref_clk, l3_clk, l4_sp_clk;

	data32 = mmio_read_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLGLOB);
	ref_clk = get_ref_clk(data32);

	l3_clk = get_l3_clk(ref_clk);

	data32 = mmio_read_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_NOCDIV);
	data32 = (data32 >> 16) & 0x3;
	data32 = 1 << data32;

	l4_sp_clk = (l3_clk / data32);

	return l4_sp_clk;
}

/* Calculate clock frequency to be used for SDMMC driver */
uint32_t get_mmc_clk(void)
{
	uint32_t data32, ref_clk, l3_clk, mmc_clk;

	data32 = mmio_read_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_PLLGLOB);
	ref_clk = get_ref_clk(data32);

	l3_clk = get_l3_clk(ref_clk);

	data32 = mmio_read_32(ALT_CLKMGR_MAINPLL + ALT_CLKMGR_MAINPLL_CNTR6CLK);
	mmc_clk = (l3_clk / (data32 + 1)) / 4;

	return mmc_clk;
}
