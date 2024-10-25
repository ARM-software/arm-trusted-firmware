/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
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
#include "socfpga_system_manager.h"

/* Main and Peripheral PLL configurations in Power Down(PD) state. */
static const pll_cfg_t pll_cfg_set[] = {
	{
		/* Enable source synchronous mode */
		CLKMGR_PLLCFG_SRC_SYNC_MODE,
		BIT(7),
		BIT(7)
	},
	{
		/* Limit the PLL overshoot frequency during lock */
		CLKMGR_PLLCFG_OVRSHOOT_FREQ_LOCK,
		BIT(0),
		BIT(0)
	},
	{
		/* To give the PLL more time to settle before lock is asserted */
		CLKMGR_PLLCFG_LOCK_SETTLE_TIME,
		BIT(0),
		BIT(0)
	},
	{
		/* To set the PLL centering duty cycle for clock slice 0 */
		CLKMGR_PLLCFG_DUTYCYCLE_CLKSLICE0,
		0x4A,
		GENMASK(6, 0)
	},
	{
		/* To set the PLL centering duty cycle for clock slice 1 */
		CLKMGR_PLLCFG_DUTYCYCLE_CLKSLICE1,
		0x4A,
		GENMASK(6, 0)
	},
};

static int wait_pll_lock(uint32_t mask)
{
	uint32_t data;
	uint32_t count = 0;
	uint32_t retry = 0U;

	do {
		/* return time out */
		if (count >= CLKMGR_MAX_RETRY_COUNT) {
			ERROR("CLKMGR: Timed out to satisfy the PLL mask\n");
			return -ETIMEDOUT;
		}

		data = mmio_read_32(CLKMGR(STAT)) & mask;
		/* wait for stable lock, make sure it is stable for these counts */
		if (data == mask) {
			retry++;
		} else {
			retry = 0U;
		}

		/* we are good now, break */
		if (retry >= 5U) {
			break;
		}

		count++;
	} while (1);

	return 0;
}

static int wait_fsm(void)
{
	uint32_t data;
	uint32_t count = 0;

	do {
		data = mmio_read_32(CLKMGR(STAT));
		count++;
		if (count >= CLKMGR_MAX_RETRY_COUNT) {
			ERROR("CLKMGR: Timed out on fsm state\n");
			return -ETIMEDOUT;
		}
	} while (CLKMGR_STAT_BUSY(data) == CLKMGR_STAT_BUSY_E_BUSY);

	return 0;
}

static uint32_t calc_pll_vcocalibration(uint32_t pllm, uint32_t pllglob)
{
	uint32_t mdiv, refclkdiv, drefclkdiv, mscnt, hscnt, vcocalib;

	mdiv = pllm & CLKMGR_PLLM_MDIV_MASK;
	drefclkdiv = ((pllglob & CLKMGR_PLLGLOB_DREFCLKDIV_MASK) >>
			CLKMGR_PLLGLOB_DREFCLKDIV_OFFSET);
	refclkdiv = ((pllglob & CLKMGR_PLLGLOB_REFCLKDIV_MASK) >>
			CLKMGR_PLLGLOB_REFCLKDIV_OFFSET);
	mscnt = CLKMGR_VCOCALIB_MSCNT_CONST / (mdiv * BIT(drefclkdiv));
	if (mscnt == 0) {
		mscnt = 1;
	}

	hscnt = (mdiv * mscnt * BIT(drefclkdiv) / refclkdiv) -
		CLKMGR_VCOCALIB_HSCNT_CONST;

	vcocalib = (hscnt & CLKMGR_VCOCALIB_HSCNT_MASK) |
			((mscnt << CLKMGR_VCOCALIB_MSCNT_OFFSET) &
			CLKMGR_VCOCALIB_MSCNT_MASK);

	return vcocalib;
}

static int pll_source_sync_wait(uint32_t pll_type, int retry_count)
{
	int count = 0;
	uint32_t req_status;

	if ((pll_type == CLKMGR_MAINPLL_TYPE) != 0) {
		req_status = mmio_read_32(CLKMGR_MAINPLL(MEM));
	} else {
		req_status = mmio_read_32(CLKMGR_PERPLL(MEM));
	}

	/* Check for error bit set */
	if ((req_status & CLKMGR_MEM_ERR) != 0) {
		INFO("CLKMGR: %s: Memory Error Status Signal Assert\n", __func__);
	}

	while ((count < retry_count) && (req_status & CLKMGR_MEM_REQ)) {
		if (pll_type == CLKMGR_MAINPLL_TYPE)
			req_status = mmio_read_32(CLKMGR_MAINPLL(MEM));
		else
			req_status = mmio_read_32(CLKMGR_PERPLL(MEM));
		count++;
	}

	if (count >= retry_count) {
		ERROR("CLKMGR: %s: timeout with pll_type %d\n", __func__, pll_type);
		return -ETIMEDOUT;
	}

	return 0;
}

static int pll_source_sync_config(uint32_t pll_type, uint32_t addr_offset,
				  uint32_t wdat, int retry_count)
{
	uint32_t addr;
	uint32_t val;

	addr = ((addr_offset | CLKMGR_MEM_ADDR_START) & CLKMGR_MEM_ADDR_MASK);
	val = (CLKMGR_MEM_REQ | CLKMGR_MEM_WR |
		(wdat << CLKMGR_MEM_WDAT_OFFSET) | addr);

	if ((pll_type == CLKMGR_MAINPLL_TYPE) != 0) {
		mmio_write_32(CLKMGR_MAINPLL(MEM), val);
	} else {
		mmio_write_32(CLKMGR_PERPLL(MEM), val);
	}

	return pll_source_sync_wait(pll_type, retry_count);
}

static int pll_source_sync_read(uint32_t pll_type, uint32_t addr_offset,
				uint32_t *rdata, int retry_count)
{
	uint32_t addr;
	uint32_t val;

	addr = ((addr_offset | CLKMGR_MEM_ADDR_START) & CLKMGR_MEM_ADDR_MASK);
	val = ((CLKMGR_MEM_REQ & ~CLKMGR_MEM_WR) | addr);

	if ((pll_type == CLKMGR_MAINPLL_TYPE) != 0) {
		mmio_write_32(CLKMGR_MAINPLL(MEM), val);
	} else {
		mmio_write_32(CLKMGR_PERPLL(MEM), val);
	}

	*rdata = 0;

	if ((pll_source_sync_wait(pll_type, retry_count)) != 0) {
		return -ETIMEDOUT;
	}

	if ((pll_type == CLKMGR_MAINPLL_TYPE) != 0) {
		*rdata = mmio_read_32(CLKMGR_MAINPLL(MEMSTAT));
	} else {
		*rdata = mmio_read_32(CLKMGR_PERPLL(MEMSTAT));
	}

	return 0;
}

static void config_pll_pd_state(uint32_t pll_type)
{
	uint32_t rdata;

	for (uint32_t i = 0; i < ARRAY_SIZE(pll_cfg_set); i++) {
		(void)pll_source_sync_read(pll_type, pll_cfg_set[i].addr, &rdata,
					   CLKMGR_MAX_RETRY_COUNT);

		(void)pll_source_sync_config(pll_type, pll_cfg_set[i].addr,
				((rdata & ~pll_cfg_set[i].mask) | pll_cfg_set[i].data),
				CLKMGR_MAX_RETRY_COUNT);
	}
}

int config_clkmgr_handoff(handoff *hoff_ptr)
{
	int ret = 0;
	uint32_t mainpll_vcocalib;
	uint32_t perpll_vcocalib;

	/* Enter boot mode before any configuration */
	mmio_setbits_32(CLKMGR(CTRL), CLKMGR_CTRL_BOOTMODE);

	/* Bypass all mainpllgrp's clocks to input clock ref */
	mmio_setbits_32(CLKMGR_MAINPLL(BYPASS), CLKMGR_MAINPLL_BYPASS_ALL);
	ret = wait_fsm();
	if (ret != 0)
		return ret;

	/* Bypass all perpllgrp's clocks to input clock ref */
	mmio_setbits_32(CLKMGR_PERPLL(BYPASS), CLKMGR_PERPLL_BYPASS_ALL);
	ret = wait_fsm();
	if (ret != 0)
		return ret;

	/* Take both PLL out of reset and power down */
	mmio_clrbits_32(CLKMGR_MAINPLL(PLLGLOB),
			CLKMGR_MAINPLL_PLLGLOB_PD_N | CLKMGR_MAINPLL_PLLGLOB_RST_N);
	mmio_clrbits_32(CLKMGR_PERPLL(PLLGLOB),
			CLKMGR_PERPLL_PLLGLOB_PD_N | CLKMGR_PERPLL_PLLGLOB_RST_N);

	/* Setup main PLL dividers */
	mainpll_vcocalib = calc_pll_vcocalibration(hoff_ptr->main_pll_pllm,
						hoff_ptr->main_pll_pllglob);
	mmio_write_32(CLKMGR_MAINPLL(PLLGLOB),
			hoff_ptr->main_pll_pllglob & ~CLKMGR_MAINPLL_PLLGLOB_RST_N);
	mmio_write_32(CLKMGR_MAINPLL(FDBCK), hoff_ptr->main_pll_fdbck);
	mmio_write_32(CLKMGR_MAINPLL(VCOCALIB), mainpll_vcocalib);
	mmio_write_32(CLKMGR_MAINPLL(PLLC0), hoff_ptr->main_pll_pllc0);
	mmio_write_32(CLKMGR_MAINPLL(PLLC1), hoff_ptr->main_pll_pllc1);
	mmio_write_32(CLKMGR_MAINPLL(PLLC2), hoff_ptr->main_pll_pllc2);
	mmio_write_32(CLKMGR_MAINPLL(PLLC3), hoff_ptr->main_pll_pllc3);
	mmio_write_32(CLKMGR_MAINPLL(PLLM), hoff_ptr->main_pll_pllm);
	mmio_write_32(CLKMGR_MAINPLL(NOCCLK), hoff_ptr->main_pll_nocclk);
	mmio_write_32(CLKMGR_MAINPLL(NOCDIV), hoff_ptr->main_pll_nocdiv);

	/* Setup peripheral PLL dividers */
	perpll_vcocalib = calc_pll_vcocalibration(hoff_ptr->per_pll_pllm,
						hoff_ptr->per_pll_pllglob);
	mmio_write_32(CLKMGR_PERPLL(PLLGLOB),
			hoff_ptr->per_pll_pllglob & ~CLKMGR_PERPLL_PLLGLOB_RST_N);
	mmio_write_32(CLKMGR_PERPLL(FDBCK), hoff_ptr->per_pll_fdbck);
	mmio_write_32(CLKMGR_PERPLL(VCOCALIB), perpll_vcocalib);
	mmio_write_32(CLKMGR_PERPLL(PLLC0), hoff_ptr->per_pll_pllc0);
	mmio_write_32(CLKMGR_PERPLL(PLLC1), hoff_ptr->per_pll_pllc1);
	mmio_write_32(CLKMGR_PERPLL(PLLC2), hoff_ptr->per_pll_pllc2);
	mmio_write_32(CLKMGR_PERPLL(PLLC3), hoff_ptr->per_pll_pllc3);
	mmio_write_32(CLKMGR_PERPLL(PLLM), hoff_ptr->per_pll_pllm);
	mmio_write_32(CLKMGR_PERPLL(EMACCTL), hoff_ptr->per_pll_emacctl);
	mmio_write_32(CLKMGR_PERPLL(GPIODIV), hoff_ptr->per_pll_gpiodiv);

	/* Configure ping pong counters */
	mmio_write_32(CLKMGR_ALTERA(EMACACTR), hoff_ptr->alt_emacactr);
	mmio_write_32(CLKMGR_ALTERA(EMACBCTR), hoff_ptr->alt_emacbctr);
	mmio_write_32(CLKMGR_ALTERA(EMACPTPCTR), hoff_ptr->alt_emacptpctr);
	mmio_write_32(CLKMGR_ALTERA(GPIODBCTR), hoff_ptr->alt_gpiodbctr);
	mmio_write_32(CLKMGR_ALTERA(S2FUSER0CTR), hoff_ptr->alt_s2fuser0ctr);
	mmio_write_32(CLKMGR_ALTERA(S2FUSER1CTR), hoff_ptr->alt_s2fuser1ctr);
	mmio_write_32(CLKMGR_ALTERA(PSIREFCTR), hoff_ptr->alt_psirefctr);
	mmio_write_32(CLKMGR_ALTERA(USB31CTR), hoff_ptr->alt_usb31ctr);
	mmio_write_32(CLKMGR_ALTERA(DSUCTR), hoff_ptr->alt_dsuctr);
	mmio_write_32(CLKMGR_ALTERA(CORE01CTR), hoff_ptr->alt_core01ctr);
	mmio_write_32(CLKMGR_ALTERA(CORE23CTR), hoff_ptr->alt_core23ctr);
	mmio_write_32(CLKMGR_ALTERA(CORE2CTR), hoff_ptr->alt_core2ctr);
	mmio_write_32(CLKMGR_ALTERA(CORE3CTR), hoff_ptr->alt_core3ctr);

	/* Take both PLL out of reset and power up */
	mmio_setbits_32(CLKMGR_MAINPLL(PLLGLOB),
			CLKMGR_MAINPLL_PLLGLOB_PD_N | CLKMGR_MAINPLL_PLLGLOB_RST_N);
	mmio_setbits_32(CLKMGR_PERPLL(PLLGLOB),
			CLKMGR_PERPLL_PLLGLOB_PD_N | CLKMGR_PERPLL_PLLGLOB_RST_N);

	/* Main PLL configuration in Powed down state */
	config_pll_pd_state(CLKMGR_MAINPLL_TYPE);

	/* Peripheral PLL configuration in Powed down state */
	config_pll_pd_state(CLKMGR_PERPLL_TYPE);

	/* Enable main PLL clkslices */
	mmio_setbits_32(CLKMGR_MAINPLL(PLLC0), CLKMGR_MAINPLL_PLLCX_EN);
	mmio_setbits_32(CLKMGR_MAINPLL(PLLC1), CLKMGR_MAINPLL_PLLCX_EN);
	mmio_setbits_32(CLKMGR_MAINPLL(PLLC2), CLKMGR_MAINPLL_PLLCX_EN);
	mmio_setbits_32(CLKMGR_MAINPLL(PLLC3), CLKMGR_MAINPLL_PLLCX_EN);

	/* Enable periheral PLL clkslices */
	mmio_setbits_32(CLKMGR_PERPLL(PLLC0), CLKMGR_PERPLL_PLLCX_EN);
	mmio_setbits_32(CLKMGR_PERPLL(PLLC1), CLKMGR_PERPLL_PLLCX_EN);
	mmio_setbits_32(CLKMGR_PERPLL(PLLC2), CLKMGR_PERPLL_PLLCX_EN);
	mmio_setbits_32(CLKMGR_PERPLL(PLLC3), CLKMGR_PERPLL_PLLCX_EN);

	/* Wait for main and peri PLL lock state */
	ret = wait_pll_lock(CLKMGR_STAT_ALLPLLLOCKED);
	if (ret != 0) {
		return ret;
	}

	/* Main PLL and per PLL lost lock */
	mmio_setbits_32(CLKMGR_MAINPLL(LOSTLOCK), CLKMGR_XPLL_LOSTLOCK_BYPASSCLEAR);
	mmio_setbits_32(CLKMGR_PERPLL(LOSTLOCK), CLKMGR_XPLL_LOSTLOCK_BYPASSCLEAR);

	/* Main PLL and per PLL clear lostlock bypass */
	mmio_setbits_32(CLKMGR_MAINPLL(PLLGLOB), CLKMGR_XPLLGLOB_CLR_LOSTLOCK_BYPASS);
	mmio_setbits_32(CLKMGR_PERPLL(PLLGLOB), CLKMGR_XPLLGLOB_CLR_LOSTLOCK_BYPASS);

	/* Pass clock source frequency into boot scratch register */
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1), hoff_ptr->hps_osc_clk_hz);
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_2), hoff_ptr->fpga_clk_hz);

	/* Take all PLLs out of bypass */
	mmio_clrbits_32(CLKMGR_MAINPLL(BYPASS), CLKMGR_MAINPLL_BYPASS_ALL);
	ret = wait_fsm();
	if (ret != 0) {
		return ret;
	}

	mmio_clrbits_32(CLKMGR_PERPLL(BYPASS), CLKMGR_PERPLL_BYPASS_ALL);
	ret = wait_fsm();
	if (ret != 0) {
		return ret;
	}

	/* Clear the loss of lock bits (write 1 to clear) */
	mmio_write_32(CLKMGR(INTRCLR),
			CLKMGR_INTRCLR_MAINLOCKLOST | CLKMGR_INTRCLR_PERLOCKLOST);

	/* Take all ping pong counters out of reset */
	mmio_clrbits_32(CLKMGR_ALTERA(EXTCNTRST), CLKMGR_ALTERA_EXTCNTRST_ALLCNTRST);

	/* Exit boot mode */
	mmio_clrbits_32(CLKMGR(CTRL), CLKMGR_CTRL_BOOTMODE);

	return 0;
}

/* Extract reference clock from platform clock source */
uint32_t get_ref_clk(uint32_t pllglob_reg, uint32_t pllm_reg)
{
	uint32_t arefclkdiv, ref_clk, mdiv;
	uint32_t pllglob_val, pllm_val;

	/* Read pllglob and pllm registers */
	pllglob_val = mmio_read_32(pllglob_reg);
	pllm_val = mmio_read_32(pllm_reg);

	switch (CLKMGR_PLLGLOB_PSRC(pllglob_val)) {
	case CLKMGR_PLLGLOB_PSRC_EOSC1:
		ref_clk = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1));
		break;

	case CLKMGR_PLLGLOB_PSRC_INTOSC:
		ref_clk = CLKMGR_INTOSC_HZ;
		break;

	case CLKMGR_PLLGLOB_PSRC_F2S:
		ref_clk = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_2));
		break;

	default:
		ref_clk = 0;
		assert(0);
		break;
	}

	/* Get reference clock divider */
	arefclkdiv = CLKMGR_PLLGLOB_AREFCLKDIV(pllglob_val);
	ref_clk /= arefclkdiv;

	/* Feedback clock divider */
	mdiv = CLKMGR_PLLM_MDIV(pllm_val);
	ref_clk *= mdiv;

	VERBOSE("CLKMGR: %s: ref_clk %u\n", __func__, ref_clk);
	return ref_clk;
}

/* Calculate clock frequency based on parameter */
uint32_t get_clk_freq(uint32_t psrc_reg, uint32_t mainpllc_reg,
		      uint32_t perpllc_reg)
{
	uint32_t clock = 0;
	uint32_t clk_psrc;

	/*
	 * Select source for the active 5:1 clock selection when the PLL
	 * is not bypassed
	 */
	clk_psrc = mmio_read_32(psrc_reg);
	switch (GET_CLKMGR_CLKSRC(clk_psrc)) {
	case CLKMGR_CLKSRC_MAIN:
		clock = get_ref_clk(CLKMGR_MAINPLL(PLLGLOB), CLKMGR_MAINPLL(PLLM));
		clock /= (mmio_read_32(mainpllc_reg) & CLKMGR_PLLCX_DIV_MSK);
		break;

	case CLKMGR_CLKSRC_PER:
		clock = get_ref_clk(CLKMGR_PERPLL(PLLGLOB), CLKMGR_PERPLL(PLLM));
		clock /= (mmio_read_32(perpllc_reg) & CLKMGR_PLLCX_DIV_MSK);
		break;

	case CLKMGR_CLKSRC_OSC1:
		clock = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1));
		break;

	case CLKMGR_CLKSRC_INTOSC:
		clock = CLKMGR_INTOSC_HZ;
		break;

	case CLKMGR_CLKSRC_FPGA:
		clock = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_2));
		break;

	default:
		ERROR("CLKMGR: Invalid clock source select\n");
		assert(0);
		break;
	}

	VERBOSE("CLKMGR: %s: clock type %lu and its value %u\n",
			__func__, GET_CLKMGR_CLKSRC(clk_psrc), clock);

	return clock;
}

/* Get L3 free clock */
static uint32_t get_l3_main_free_clk(void)
{
	return get_clk_freq(CLKMGR_MAINPLL(NOCCLK),
			    CLKMGR_MAINPLL(PLLC3),
			    CLKMGR_PERPLL(PLLC1));
}

/* Get L4 main clock */
static uint32_t get_l4_main_clk(void)
{
	return get_l3_main_free_clk();
}

/* Get L4 mp clock */
static uint32_t get_l4_mp_clk(void)
{
	uint32_t l3_main_free_clk = get_l3_main_free_clk();
	uint32_t mainpll_nocdiv_l4mp = BIT(GET_CLKMGR_MAINPLL_NOCDIV_L4MP(
					mmio_read_32(CLKMGR_MAINPLL(NOCDIV))));

	uint32_t l4_mp_clk = (l3_main_free_clk / mainpll_nocdiv_l4mp);

	return l4_mp_clk;
}

/* Get L4 sp clock */
static uint32_t get_l4_sp_clk(void)
{
	uint32_t l3_main_free_clk = get_l3_main_free_clk();
	uint32_t mainpll_nocdiv_l4sp = BIT(GET_CLKMGR_MAINPLL_NOCDIV_L4SP(
					mmio_read_32(CLKMGR_MAINPLL(NOCDIV))));

	uint32_t l4_sp_clk = (l3_main_free_clk / mainpll_nocdiv_l4sp);

	return l4_sp_clk;
}

/* Calculate clock frequency to be used for SDMMC driver */
uint32_t get_sdmmc_clk(void)
{
	uint32_t l4_mp_clk = get_l4_mp_clk();
	uint32_t mainpll_nocdiv = mmio_read_32(CLKMGR_MAINPLL(NOCDIV));
	uint32_t sdmmc_clk = l4_mp_clk / BIT(GET_CLKMGR_MAINPLL_NOCDIV_SPHY(mainpll_nocdiv));

	return sdmmc_clk;
}

/* Get clock for ethernet mac0 */
static uint32_t get_emaca_clk(void)
{
	uint32_t emaca_ctr = mmio_read_32(CLKMGR_ALTERA(EMACACTR));
	uint32_t perpll_emacctl = mmio_read_32(CLKMGR_PERPLL(EMACCTL));
	uint32_t perpll_emac_src = GET_CLKMGR_PERPLL_EMAC0_CLK_SRC(perpll_emacctl);
	uint32_t emac_ctr_reg;
	uint32_t emac_clock;

	if (perpll_emac_src != 0) {
		emac_ctr_reg = CLKMGR_ALTERA(EMACBCTR);
	} else {
		emac_ctr_reg = CLKMGR_ALTERA(EMACACTR);
	}

	/* Get EMACA clock source */
	uint32_t emacactr_src = GET_CLKMGR_EMACACTR_CLK_SRC(emaca_ctr);

	/* Read the desired EMAC register again */
	emaca_ctr = mmio_read_32(emac_ctr_reg);

	/* Get the divider now */
	uint32_t emaca_ctr_div = emaca_ctr & GENMASK(10, 0);

	switch (emacactr_src) {
	case CLKMGR_CLKSRC_MAIN:
		emac_clock = get_ref_clk(CLKMGR_MAINPLL(PLLGLOB), CLKMGR_MAINPLL(PLLM));
		emac_clock /= (mmio_read_32(CLKMGR_MAINPLL(PLLC1)) & GENMASK(10, 0));
		break;

	case CLKMGR_CLKSRC_PER:
		emac_clock = get_ref_clk(CLKMGR_PERPLL(PLLGLOB), CLKMGR_PERPLL(PLLM));
		emac_clock /= (mmio_read_32(CLKMGR_PERPLL(PLLC3)) & GENMASK(10, 0));
		break;

	default:
		ERROR("CLKMGR: %s invalid clock source\n", __func__);
		emac_clock = 0;
		return emac_clock;
	}

	emac_clock /= 1 + emaca_ctr_div;

	return emac_clock;
}

/* Get MPU clock */
static uint32_t get_mpu_clk(void)
{
	uint32_t cpu_id = MPIDR_AFFLVL1_VAL(read_mpidr());
	uint32_t ctr_reg = 0U;
	uint32_t clock;

	if (cpu_id > CLKMGR_ALTERA_CORE1) {
		clock = get_clk_freq(CLKMGR_ALTERA(CORE23CTR),
				     CLKMGR_MAINPLL(PLLC0),
				     CLKMGR_PERPLL(PLLC0));
	} else {
		clock = get_clk_freq(CLKMGR_ALTERA(CORE01CTR),
				     CLKMGR_MAINPLL(PLLC1),
				     CLKMGR_PERPLL(PLLC0));
	}

	switch (cpu_id) {
	case CLKMGR_ALTERA_CORE0:
	case CLKMGR_ALTERA_CORE1:
		ctr_reg = CLKMGR_ALTERA(CORE01CTR);
		break;

	case CLKMGR_ALTERA_CORE2:
		ctr_reg = CLKMGR_ALTERA(CORE2CTR);
		break;

	case CLKMGR_ALTERA_CORE3:
		ctr_reg = CLKMGR_ALTERA(CORE3CTR);
		break;

	default:
		break;
	}

	/* Division setting for ping pong counter in clock slice */
	clock /= 1 + (mmio_read_32(ctr_reg) & CLKMGR_PLLCX_DIV_MSK);

	return clock;
}

/* Calculate clock frequency to be used for watchdog timer */
static uint32_t get_wdt_clk(void)
{
	uint32_t l3_main_free_clk = get_l3_main_free_clk();
	uint32_t mainpll_nocdiv_l4sysfreeclk = BIT(GET_CLKMGR_MAINPLL_NOCDIV_L4SYSFREE(
						mmio_read_32(CLKMGR_MAINPLL(NOCDIV))));
	uint32_t l4_sys_free_clk = (l3_main_free_clk / mainpll_nocdiv_l4sysfreeclk);

	return l4_sys_free_clk;
}

/*
 * Calculate clock frequency to be used for UART driver.
 * 'l4_sp_clk' (100MHz) will be used for slow peripherals like UART, I2C
 * and Timers.
 */
static uint32_t get_uart_clk(void)
{
	return get_l4_sp_clk();
}

/* Return the clock value of a given system component */
uint32_t clkmgr_get_rate(uint32_t clk_id)
{
	uint32_t clk_rate;

	switch (clk_id) {
	case CLKMGR_MPU_CLK_ID:
		clk_rate = get_mpu_clk();
		break;

	case CLKMGR_L4_MAIN_CLK_ID:
		clk_rate = get_l4_main_clk();
		break;

	case CLKMGR_L4_MP_CLK_ID:
		clk_rate = get_l4_mp_clk();
		break;

	case CLKMGR_L4_SP_CLK_ID:
		clk_rate = get_l4_sp_clk();
		break;

	case CLKMGR_EMAC0_CLK_ID:
		clk_rate = get_emaca_clk();
		break;

	case CLKMGR_SDMMC_CLK_ID:
		clk_rate = get_sdmmc_clk();
		break;

	case CLKMGR_UART_CLK_ID:
		clk_rate = get_uart_clk();
		break;

	case CLKMGR_WDT_CLK_ID:
		clk_rate = get_wdt_clk();
		break;

	default:
		ERROR("CLKMGR: %s: Invalid clock ID\n", __func__);
		clk_rate = 0;
		break;
	}

	return clk_rate;
}

/* Return mpu_periph_clk tick */
unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_SYS_COUNTER_FREQ_IN_TICKS;
}
