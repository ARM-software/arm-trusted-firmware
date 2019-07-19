/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>

#include "agilex_clock_manager.h"
#include "agilex_handoff.h"

static const CLOCK_SOURCE_CONFIG  clk_source = {
	/* clk_freq_of_eosc1 */
	(uint32_t) 25000000,
	/* clk_freq_of_f2h_free */
	(uint32_t) 400000000,
	/* clk_freq_of_cb_intosc_ls */
	(uint32_t) 50000000,
};

uint32_t wait_pll_lock(void)
{
	uint32_t data;
	uint32_t count = 0;

	do {
		data = mmio_read_32(CLKMGR_OFFSET + CLKMGR_STAT);
		count++;
		if (count >= 1000)
			return -ETIMEDOUT;

	} while ((CLKMGR_STAT_MAINPLLLOCKED(data) == 0) ||
			(CLKMGR_STAT_PERPLLLOCKED(data) == 0));
	return 0;
}

uint32_t wait_fsm(void)
{
	uint32_t data;
	uint32_t count = 0;

	do {
		data = mmio_read_32(CLKMGR_OFFSET + CLKMGR_STAT);
		count++;
		if (count >= 1000)
			return -ETIMEDOUT;

	} while (CLKMGR_STAT_BUSY(data) == CLKMGR_STAT_BUSY_E_BUSY);

	return 0;
}

uint32_t pll_source_sync_config(uint32_t pll_mem_offset)
{
	uint32_t val = 0;
	uint32_t count = 0;
	uint32_t req_status = 0;

	val = (CLKMGR_MEM_WR | CLKMGR_MEM_REQ |
		CLKMGR_MEM_WDAT << CLKMGR_MEM_WDAT_OFFSET | CLKMGR_MEM_ADDR);
	mmio_write_32(pll_mem_offset, val);

	do {
		req_status = mmio_read_32(pll_mem_offset);
		count++;
	} while ((req_status & CLKMGR_MEM_REQ) && (count < 10));

	if (count >= 100)
		return -ETIMEDOUT;

	return 0;
}

uint32_t pll_source_sync_read(uint32_t pll_mem_offset)
{
	uint32_t val = 0;
	uint32_t rdata = 0;
	uint32_t count = 0;
	uint32_t req_status = 0;

	val = (CLKMGR_MEM_REQ | CLKMGR_MEM_ADDR);
	mmio_write_32(pll_mem_offset, val);

	do {
		req_status = mmio_read_32(pll_mem_offset);
		count++;
	} while ((req_status & CLKMGR_MEM_REQ) && (count < 10));

	if (count >= 100)
		return -ETIMEDOUT;

	rdata = mmio_read_32(pll_mem_offset + 0x4);
	INFO("rdata (%x) = %x\n", pll_mem_offset + 0x4, rdata);

	return 0;
}

void config_clkmgr_handoff(handoff *hoff_ptr)
{
	uint32_t mdiv, mscnt, hscnt;
	uint32_t arefclk_div, drefclk_div;

	/* Bypass all mainpllgrp's clocks */
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_BYPASS, 0x7);
	wait_fsm();

	/* Bypass all perpllgrp's clocks */
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_BYPASS, 0x7f);
	wait_fsm();

	/* Put both PLL in reset and power down */
	mmio_clrbits_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLGLOB,
			CLKMGR_MAINPLL_PLLGLOB_PD_SET_MSK |
			CLKMGR_MAINPLL_PLLGLOB_RST_SET_MSK);
	mmio_clrbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLGLOB,
			CLKMGR_PERPLL_PLLGLOB_PD_SET_MSK |
			CLKMGR_PERPLL_PLLGLOB_RST_SET_MSK);

	/* Setup main PLL dividers */
	mdiv = CLKMGR_MAINPLL_PLLM_MDIV(hoff_ptr->main_pll_pllm);

	arefclk_div = CLKMGR_MAINPLL_PLLGLOB_AREFCLKDIV(
			hoff_ptr->main_pll_pllglob);
	drefclk_div = CLKMGR_MAINPLL_PLLGLOB_DREFCLKDIV(
			hoff_ptr->main_pll_pllglob);

	mscnt = 100 / (mdiv / BIT(drefclk_div));
	if (!mscnt)
		mscnt = 1;
	hscnt = (mdiv * mscnt * BIT(drefclk_div) / arefclk_div) - 4;

	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_VCOCALIB,
			CLKMGR_MAINPLL_VCOCALIB_HSCNT_SET(hscnt) |
			CLKMGR_MAINPLL_VCOCALIB_MSCNT_SET(mscnt));

	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_NOCDIV,
			hoff_ptr->main_pll_nocdiv);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLGLOB,
			hoff_ptr->main_pll_pllglob);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_FDBCK,
			hoff_ptr->main_pll_fdbck);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLC0,
			hoff_ptr->main_pll_pllc0);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLC1,
			hoff_ptr->main_pll_pllc1);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLC2,
			hoff_ptr->main_pll_pllc2);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLC3,
			hoff_ptr->main_pll_pllc3);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLM,
			hoff_ptr->main_pll_pllm);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_MPUCLK,
			hoff_ptr->main_pll_mpuclk);
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_NOCCLK,
			hoff_ptr->main_pll_nocclk);

	/* Setup peripheral PLL dividers */
	mdiv = CLKMGR_PERPLL_PLLM_MDIV(hoff_ptr->per_pll_pllm);

	arefclk_div = CLKMGR_PERPLL_PLLGLOB_AREFCLKDIV(
			hoff_ptr->per_pll_pllglob);
	drefclk_div = CLKMGR_PERPLL_PLLGLOB_DREFCLKDIV(
			hoff_ptr->per_pll_pllglob);

	mscnt = 100 / (mdiv / BIT(drefclk_div));
	if (!mscnt)
		mscnt = 1;
	hscnt = (mdiv * mscnt * BIT(drefclk_div) / arefclk_div) - 4;

	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_VCOCALIB,
			CLKMGR_PERPLL_VCOCALIB_HSCNT_SET(hscnt) |
			CLKMGR_PERPLL_VCOCALIB_MSCNT_SET(mscnt));

	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EMACCTL,
			hoff_ptr->per_pll_emacctl);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_GPIODIV,
			CLKMGR_PERPLL_GPIODIV_GPIODBCLK_SET(
			hoff_ptr->per_pll_gpiodiv));
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLGLOB,
			hoff_ptr->per_pll_pllglob);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_FDBCK,
			hoff_ptr->per_pll_fdbck);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLC0,
			hoff_ptr->per_pll_pllc0);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLC1,
			hoff_ptr->per_pll_pllc1);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLC2,
			hoff_ptr->per_pll_pllc2);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLC3,
			hoff_ptr->per_pll_pllc3);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLM,
			hoff_ptr->per_pll_pllm);

	/* Take both PLL out of reset and power up */
	mmio_setbits_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLGLOB,
			CLKMGR_MAINPLL_PLLGLOB_PD_SET_MSK |
			CLKMGR_MAINPLL_PLLGLOB_RST_SET_MSK);
	mmio_setbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLGLOB,
			CLKMGR_PERPLL_PLLGLOB_PD_SET_MSK |
			CLKMGR_PERPLL_PLLGLOB_RST_SET_MSK);

	wait_pll_lock();

	pll_source_sync_config(CLKMGR_MAINPLL + CLKMGR_MAINPLL_MEM);
	pll_source_sync_read(CLKMGR_MAINPLL + CLKMGR_MAINPLL_MEM);

	pll_source_sync_config(CLKMGR_PERPLL + CLKMGR_PERPLL_MEM);
	pll_source_sync_read(CLKMGR_PERPLL + CLKMGR_PERPLL_MEM);

	/*Configure Ping Pong counters in altera group */
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_EMACACTR,
			hoff_ptr->alt_emacactr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_EMACBCTR,
			hoff_ptr->alt_emacbctr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_EMACPTPCTR,
			hoff_ptr->alt_emacptpctr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_GPIODBCTR,
			hoff_ptr->alt_gpiodbctr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_SDMMCCTR,
			hoff_ptr->alt_sdmmcctr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_S2FUSER0CTR,
			hoff_ptr->alt_s2fuser0ctr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_S2FUSER1CTR,
			hoff_ptr->alt_s2fuser1ctr);
	mmio_write_32(CLKMGR_ALTERA + CLKMGR_ALTERA_PSIREFCTR,
			hoff_ptr->alt_psirefctr);

	/* Take all PLLs out of bypass */
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_BYPASS, 0);
	wait_fsm();

	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_BYPASS, 0);
	wait_fsm();

	/* Clear loss lock  interrupt status register that */
	/* might be set during configuration */
	mmio_setbits_32(CLKMGR_OFFSET + CLKMGR_INTRCLR,
			CLKMGR_INTRCLR_MAINLOCKLOST_SET_MSK |
			CLKMGR_INTRCLR_PERLOCKLOST_SET_MSK);

	/* Take all ping pong counters out of reset */
	mmio_clrbits_32(CLKMGR_ALTERA + CLKMGR_ALTERA_EXTCNTRST,
			CLKMGR_ALTERA_EXTCNTRST_RESET);

	/* Set safe mode / out of boot mode */
	mmio_clrbits_32(CLKMGR_OFFSET + CLKMGR_CTRL,
		CLKMGR_CTRL_BOOTMODE_SET_MSK);
	wait_fsm();

	/* Enable mainpllgrp's software-managed clock */
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_EN,
			CLKMGR_MAINPLL_EN_RESET);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EN,
			CLKMGR_PERPLL_EN_RESET);
}

int get_wdt_clk(handoff *hoff_ptr)
{
	int main_noc_base_clk, l3_main_free_clk, l4_sys_free_clk;
	int data32, mdiv, arefclkdiv, ref_clk;

	data32 = mmio_read_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLGLOB);

	switch (CLKMGR_MAINPLL_PLLGLOB_PSRC(data32)) {
	case CLKMGR_MAINPLL_PLLGLOB_PSRC_EOSC1:
		ref_clk = clk_source.clk_freq_of_eosc1;
		break;
	case CLKMGR_MAINPLL_PLLGLOB_PSRC_INTOSC:
		ref_clk = clk_source.clk_freq_of_cb_intosc_ls;
		break;
	case CLKMGR_MAINPLL_PLLGLOB_PSRC_F2S:
		ref_clk = clk_source.clk_freq_of_f2h_free;
		break;
	default:
		ref_clk = 0;
		assert(0);
		break;
	}

	arefclkdiv = CLKMGR_MAINPLL_PLLGLOB_AREFCLKDIV(data32);
	mdiv = CLKMGR_MAINPLL_PLLM_MDIV(hoff_ptr->main_pll_pllm);

	ref_clk = (ref_clk / arefclkdiv) * mdiv;
	main_noc_base_clk = ref_clk / (hoff_ptr->main_pll_pllc1 & 0x7ff);
	l3_main_free_clk = main_noc_base_clk / (hoff_ptr->main_pll_nocclk + 1);
	l4_sys_free_clk = l3_main_free_clk / 4;

	return l4_sys_free_clk;
}
