/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "agilex5_clock_manager.h"
#include "agilex5_system_manager.h"
#include "socfpga_handoff.h"

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

uint32_t pll_source_sync_config(uint32_t pll_mem_offset, uint32_t data)
{
	uint32_t val = 0;
	uint32_t count = 0;
	uint32_t req_status = 0;

	val = (CLKMGR_MEM_WR | CLKMGR_MEM_REQ |
		(data << CLKMGR_MEM_WDAT_OFFSET) | CLKMGR_MEM_ADDR);
	mmio_write_32(pll_mem_offset, val);

	do {
		req_status = mmio_read_32(pll_mem_offset);
		count++;
	} while ((req_status & CLKMGR_MEM_REQ) && (count < 10));

	if (count >= 10)
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

	if (count >= 10)
		return -ETIMEDOUT;

	rdata = mmio_read_32(pll_mem_offset + 0x4);
	INFO("rdata (%x) = %x\n", pll_mem_offset + 0x4, rdata);

	return rdata;
}

void config_clkmgr_handoff(handoff *hoff_ptr)
{
	/* Take both PLL out of reset and power up */

	mmio_setbits_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLGLOB,
			CLKMGR_PLLGLOB_PD_SET_MSK |
			CLKMGR_PLLGLOB_RST_SET_MSK);
	mmio_setbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_PLLGLOB,
			CLKMGR_PLLGLOB_PD_SET_MSK |
			CLKMGR_PLLGLOB_RST_SET_MSK);

	/* PLL lock */
	wait_pll_lock();

	/* Bypass all mainpllgrp's clocks to input clock ref */
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_BYPASSS, 0xff);
	/* Bypass all perpllgrp's clocks to input clock ref */
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_BYPASS, 0xff);

	/* Pass clock source frequency into scratch register */
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1),
		hoff_ptr->hps_osc_clk_hz);
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_2),
		hoff_ptr->fpga_clk_hz);

	/* Take all PLLs out of bypass */
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_BYPASS, 0);
	wait_fsm();
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_BYPASS, 0);
	wait_fsm();

	/* Enable mainpllgrp's software-managed clock */
	mmio_write_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_EN,
			CLKMGR_MAINPLL_EN_RESET);
	mmio_write_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EN,
			CLKMGR_PERPLL_EN_RESET);
}

/* Extract reference clock from platform clock source */
uint32_t get_ref_clk(uint32_t pllglob)
{
	uint32_t arefclkdiv, ref_clk;
	uint32_t scr_reg;

	switch (CLKMGR_PSRC(pllglob)) {
	case CLKMGR_PLLGLOB_PSRC_EOSC1:
		scr_reg = SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1);
		ref_clk = mmio_read_32(scr_reg);
		break;
	case CLKMGR_PLLGLOB_PSRC_INTOSC:
		ref_clk = CLKMGR_INTOSC_HZ;
		break;
	case CLKMGR_PLLGLOB_PSRC_F2S:
		scr_reg = SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_2);
		ref_clk = mmio_read_32(scr_reg);
		break;
	default:
		ref_clk = 0;
		assert(0);
		break;
	}

	arefclkdiv = CLKMGR_PLLGLOB_AREFCLKDIV(pllglob);
	ref_clk /= arefclkdiv;

	return ref_clk;
}

/* Calculate clock frequency based on parameter */
uint32_t get_clk_freq(uint32_t psrc_reg, uint32_t main_pllc, uint32_t per_pllc)
{
	uint32_t ref_clk = 0;

	uint32_t clk_psrc, mdiv;
	uint32_t pllm_reg, pllc_reg, pllc_div, pllglob_reg;


	clk_psrc = mmio_read_32(CLKMGR_MAINPLL + psrc_reg);
	clk_psrc = 0;

	switch (clk_psrc) {
	case 0:
		pllm_reg = CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLM;
		pllc_reg = CLKMGR_MAINPLL + main_pllc;
		pllglob_reg = CLKMGR_MAINPLL + CLKMGR_MAINPLL_PLLGLOB;
		break;
	}

	ref_clk = get_ref_clk(mmio_read_32(pllglob_reg));
	mdiv = CLKMGR_PLLM_MDIV(mmio_read_32(pllm_reg));
	ref_clk *= mdiv;

	pllc_div = mmio_read_32(pllc_reg) & 0x7ff;
	NOTICE("return = %d Hz\n", (ref_clk / pllc_div));

	ref_clk = 200000000;
	return (uint32_t) ref_clk;

}

/* Return L3 interconnect clock */
uint32_t get_l3_clk(void)
{
	uint32_t l3_clk;

	l3_clk = get_clk_freq(CLKMGR_MAINPLL_NOCCLK, CLKMGR_MAINPLL_PLLC1,
				CLKMGR_PERPLL_PLLC1);
	return l3_clk;
}

/* Calculate clock frequency to be used for watchdog timer */
uint32_t get_wdt_clk(void)
{
	uint32_t l3_clk, l4_sys_clk;

	l3_clk = get_l3_clk();
	l4_sys_clk = l3_clk / 4;

	return l4_sys_clk;
}

/* Calculate clock frequency to be used for UART driver */
uint32_t get_uart_clk(void)
{
	uint32_t data32, l3_clk, l4_sp_clk;

	l3_clk = get_l3_clk();

	data32 = mmio_read_32(CLKMGR_MAINPLL + CLKMGR_MAINPLL_NOCDIV);
	data32 = (data32 >> 16) & 0x3;

	l4_sp_clk = l3_clk >> data32;

	return l4_sp_clk;
}

/* Calculate clock frequency to be used for SDMMC driver */
uint32_t get_mmc_clk(void)
{
	uint32_t mmc_clk;

	//TODO: To update when handoff data is ready
	//uint32_t data32;

	//mmc_clk = get_clk_freq(CLKMGR_ALTERA_SDMMCCTR, CLKMGR_MAINPLL_PLLC3, CLKMGR_PERPLL_PLLC3);

	//data32 = mmio_read_32(CLKMGR_ALTERA + CLKMGR_ALTERA_SDMMCCTR);
	//data32 = (data32 & 0x7ff) + 1;
	//mmc_clk = (mmc_clk / data32) / 4;


	mmc_clk = get_clk_freq(CLKMGR_MAINPLL_NOCCLK, CLKMGR_MAINPLL_PLLC3,
				CLKMGR_PERPLL_PLLC3);

	// TODO: To update when handoff data is ready
	NOTICE("mmc_clk = %d Hz\n", mmc_clk);

	return mmc_clk;
}
