/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>
#include <ddrphy_wrapper.h>

#include <lib/mmio.h>

#include <platform_def.h>

struct phyinit_timings {
	int tstaoff;
	int tpdm;
	int tcasl_add;
};

static struct phyinit_timings timings;

/*
 * Program dfimrl according to this formula:
 *
 *         dfimrl = ceiling( (ARdPtrinitval*UI + phy_tx_insertion_dly +
 *                            phy_rx_insertion_dly + PHY_Rx_Fifo_dly + tDQSCK + tstaoff) /
 *                           dficlk_period)
 *
 * All terms in above equation specified in ps
 * tDQSCK - determine from memory model
 * tstaoff - determine from memory model
 * phy_tx_insertion_dly = 200ps
 * phy_rx_insertion_dly = 200ps
 * phy_rx_fifo_dly      = 200ps + 4UI
 */
static void dfimrl_program(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d,
			   int ardptrinitval)
{
	uint32_t byte;
	int dfimrl_in_dficlk;
	int phy_rx_fifo_dly;
	int phy_rx_insertion_dly = 200;
	int phy_tx_insertion_dly = 200;
	long long dficlk_period_x1000;
	long long dfimrl_in_fs;
	long long uifs;
	uint16_t dfimrl;

	uifs = (1000 * 1000000) / ((int)config->uib.frequency * 2);
	dficlk_period_x1000 = 4 * uifs;

	phy_rx_fifo_dly = (int)(((200 * 1000) + (4 * uifs)) / 1000);

	dfimrl_in_fs = (ardptrinitval * uifs) +
		       ((phy_tx_insertion_dly + phy_rx_insertion_dly + phy_rx_fifo_dly +
			 timings.tstaoff + timings.tcasl_add + timings.tpdm) * 1000);

	dfimrl_in_dficlk = (int)(dfimrl_in_fs / dficlk_period_x1000);
	if ((dfimrl_in_fs % dficlk_period_x1000) != 0) {
		dfimrl_in_dficlk++;
	}
	dfimrl = (uint16_t)(dfimrl_in_dficlk + mb_ddr_1d->dfimrlmargin);

	/*
	 * mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDBYTE | CBRD | CSR_DFIMRL_ADDR))),
	 *               dfimrl);
	 */
	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;

		c_addr = byte << 12;
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDBYTE | c_addr |
								CSR_DFIMRL_ADDR))),
			      dfimrl);
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTMRL_ADDR))), dfimrl);
}

/*
 * Program txdqsdlytg0/1[9:0]:
 *
 *         txdqsdlytg*[9:6] = floor( (4*UI + tstaoff) / UI)
 *         txdqsdlytg*[5:0] = ceiling( (tstaoff%UI / UI) * 32)
 *
 * tstaoff and UI expressed in ps
 *
 * For HMD and LPDDR4X and MEMCLK <= 533 mhz:
 *    txdqsdlytg*[9:6] = 0x5
 *
 * For other dimm types, leave TDqsDlyTg*[9:0] at default 0x100
 *
 * ppp_0001_cccc_uuuu_1101_0000
 *
 * if DDR3 or DDR4
 *      num_timingroup = numrank_dfi0;
 * else
 *      num_timingroup = numrank_dfi0 + numrank_dfi1 * dfi1exists;
 */
static void txdqsdlytg_program(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d,
			       uint16_t *txdqsdly)
{
	uint32_t byte;
	int txdqsdlytg_5to0; /* Fine delay - 1/32UI per increment */
	int txdqsdlytg_9to6; /* Coarse delay - 1UI per increment */
	int txdqsdlytg_fine_default = 0;
	int txdqsdlytg_coarse_default = 4;
	long long tmp_value;
	long long uifs;

	uifs = (1000 * 1000000) / ((int)config->uib.frequency * 2);

	txdqsdlytg_9to6 = (int)(((int)((txdqsdlytg_coarse_default * uifs) / 1000) +
				 timings.tstaoff + timings.tcasl_add
				 - timings.tpdm) / (int)(uifs / 1000));

	tmp_value = fmodll(((txdqsdlytg_fine_default * uifs / 32) +
			    ((timings.tstaoff + timings.tcasl_add) * 1000) -
			    (timings.tpdm * 1000)),
			   uifs);
	txdqsdlytg_5to0 = (int)(tmp_value / uifs * 32);
	if ((tmp_value % uifs) != 0) {
		txdqsdlytg_5to0++;
	}

	/* Bit-5 of LCDL is no longer used, so bumping bit-5 of fine_dly up to coarse_dly */
	if (txdqsdlytg_5to0 >= 32) {
		txdqsdlytg_9to6 = txdqsdlytg_9to6 + 1;
		txdqsdlytg_5to0 = txdqsdlytg_5to0 - 32;
	}

	*txdqsdly = (uint16_t)((txdqsdlytg_9to6 << 6) | txdqsdlytg_5to0);

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t nibble;

		c_addr = byte << 12;
		for (nibble = 0U; nibble < 2U; nibble++) {
			uint32_t u_addr;

			if (ddrphy_phyinit_isdbytedisabled(config, mb_ddr_1d, byte) != 0) {
				continue;
			}

			u_addr = nibble << 8;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
			if ((mb_ddr_1d->cspresent & 0x1U) != 0U) {
#else /* STM32MP_LPDDR4_TYPE */
			if (((mb_ddr_1d->cspresentcha & 0x1U) |
			     (mb_ddr_1d->cspresentchb & 0x1U)) != 0U) {
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr | u_addr |
								     CSR_TXDQSDLYTG0_ADDR))),
					      *txdqsdly);
			}

#if STM32MP_LPDDR4_TYPE
			if ((((mb_ddr_1d->cspresentcha & 0x2U) >> 1) |
			     ((mb_ddr_1d->cspresentchb & 0x2U) >> 1)) != 0U) {
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr | u_addr |
								     CSR_TXDQSDLYTG1_ADDR))),
					      *txdqsdly);
			}
#endif /* STM32MP_LPDDR4_TYPE */
		}
	}
}

/*
 * ##############################################################
 *
 * Program txdqdlyTg0/1[8:0]:
 *
 *     txdqdlyTg*[8:6] = floor( (txdqsdlytg*[5:0]*UI/32 + tDQS2DQ + 0.5UI) / UI)
 *     txdqdlyTg*[5:0] = ceil( ((txdqsdlytg*[5:0]*UI/32 + tDQS2DQ + 0.5UI)%UI / UI) * 32)
 *
 * ##############################################################
 */
static void txdqdlytg_program(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d,
			      uint16_t txdqsdly)
{
	uint32_t byte;
	int txdqdly_5to0; /* Fine delay - 1/32UI per increment */
	int txdqdly_8to6; /* Coarse delay - 1UI per increment */
	int txdqsdlytg_5to0; /* Fine delay - 1/32UI per increment */
	long long tmp_value;
	long long uifs;
	uint16_t txdqdly;

	uifs = (1000 * 1000000) / ((int)config->uib.frequency * 2);

	txdqsdlytg_5to0 = (int)txdqsdly & 0x3F;

	txdqdly_8to6 = (int)(((txdqsdlytg_5to0 * uifs / 32) + (uifs / 2)) / uifs);
	tmp_value = fmodll(((txdqsdlytg_5to0 * uifs / 32) + (uifs / 2)), uifs);
	txdqdly_5to0 = (int)(((tmp_value * 32) / uifs));
	if ((tmp_value % uifs) != 0) {
		txdqdly_5to0++;
	}

	/* Bit-5 of LCDL is no longer used, so bumping bit-5 of fine_dly up to coarse_dly */
	if (txdqdly_5to0 >= 32) {
		txdqdly_8to6 = txdqdly_8to6 + 1;
		txdqdly_5to0 = txdqdly_5to0 - 32;
	}

	txdqdly = (uint16_t)((txdqdly_8to6 << 6) | txdqdly_5to0);

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t lane;

		c_addr = byte << 12;
		for (lane = 0U; lane < 9U; lane++) {
			uint32_t r_addr;

			if (ddrphy_phyinit_isdbytedisabled(config, mb_ddr_1d, byte) != 0) {
				continue;
			}

			r_addr = lane << 8;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
			if ((mb_ddr_1d->cspresent & 0x1U) != 0U) {
#else /* STM32MP_LPDDR4_TYPE */
			if (((mb_ddr_1d->cspresentcha & 0x1U) |
			     (mb_ddr_1d->cspresentchb & 0x1U)) != 0U) {
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr | r_addr |
								     CSR_TXDQDLYTG0_ADDR))),
					      txdqdly);
			}

#if STM32MP_LPDDR4_TYPE
			if ((((mb_ddr_1d->cspresentcha & 0x2U) >> 1) |
			     ((mb_ddr_1d->cspresentchb & 0x2U) >> 1)) != 0U) {
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr | r_addr |
								     CSR_TXDQDLYTG1_ADDR))),
					      txdqdly);
			}
#endif /* STM32MP_LPDDR4_TYPE */
		}
	}
}

/*
 * Program rxendly0/1[10:0]:
 *
 *         rxendly[10:6] = floor( (4*UI + tDQSCK + tstaoff) / UI)
 *         rxendly[5:0]  = ceil( ((tDQSCK + tstaoff) % UI) * 32)
 *
 * tDQSCK, tstaoff and UI expressed in ps
 */
static void rxendly_program(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d)
{
	int rxendly_coarse_default = 4;
	int rxendly_fine_default = 0;

	int backoff_x1000 __maybe_unused;
	int zerobackoff_x1000 __maybe_unused;
	uint32_t byte;
	int rxendly_10to6; /* Coarse delay - 1UI per increment */
	int rxendly_5to0; /* Fine delay - 1/32UI per increment */
	int totfinestep;
	long long finestepfs; /* Fine steps in fs */
	long long rxendly_offset_x1000000 = 0; /* 0 Offset is 1UI before the first DQS. */
	long long totfs;
	long long uifs;
	uint16_t rxendly;

	uifs = (1000 * 1000000) / ((int)config->uib.frequency * 2);

#if STM32MP_LPDDR4_TYPE
	/* Compensate for pptenrxenbackoff */
	zerobackoff_x1000 = (1000 * 24) / 32;
	if (config->uia.lp4rxpreamblemode == 1U) {
		backoff_x1000 = 1000 - ((1000 * 2) / 32);
	} else {
		backoff_x1000 = (1000 * (int)config->uia.rxenbackoff) - ((1000 * 2) / 32);
	}

	if (config->uia.disableretraining == 0U) {
		rxendly_offset_x1000000 = config->uib.frequency < 333U ?
					  backoff_x1000 * uifs : zerobackoff_x1000 * uifs;
	} else {
		rxendly_offset_x1000000 = zerobackoff_x1000 * uifs;
	}
#endif /* STM32MP_LPDDR4_TYPE */

	finestepfs = uifs / 32;
	totfs = ((32 * rxendly_coarse_default * finestepfs) +
		 (rxendly_fine_default * finestepfs) +
		 ((timings.tstaoff + timings.tcasl_add +
		   timings.tpdm) * 1000) + (rxendly_offset_x1000000 / 1000));
	totfinestep = totfs / finestepfs;

	rxendly_10to6 = totfinestep / 32;
	rxendly_5to0  = fmodi(totfinestep, 32);

	/* Bit-5 of LCDL is no longer used, so bumping bit-5 of fine_dly up to coarse_dly */
	if (rxendly_5to0 >= 32) {
		rxendly_10to6 = rxendly_10to6 + 1;
		rxendly_5to0 = rxendly_5to0 - 32;
	}

	rxendly = (uint16_t)((rxendly_10to6 << 6) | rxendly_5to0);

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t nibble;

		c_addr = byte << 12;
		for (nibble = 0U; nibble < 2U; nibble++) {
			uint32_t u_addr;

			if (ddrphy_phyinit_isdbytedisabled(config, mb_ddr_1d, byte) != 0) {
				continue;
			}

			u_addr = nibble << 8;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
			if ((mb_ddr_1d->cspresent & 0x1U) != 0) {
#else /* STM32MP_LPDDR4_TYPE */
			if (((mb_ddr_1d->cspresentcha & 0x1U) |
			     (mb_ddr_1d->cspresentchb & 0x1U)) != 0U) {
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr | u_addr |
								     CSR_RXENDLYTG0_ADDR))),
					      rxendly);
			}

#if STM32MP_LPDDR4_TYPE
			if ((((mb_ddr_1d->cspresentcha & 0x2U) >> 1) |
			     ((mb_ddr_1d->cspresentchb & 0x2U) >> 1)) != 0U) {
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr | u_addr |
								     CSR_RXENDLYTG1_ADDR))),
					      rxendly);
			}
#endif /* STM32MP_LPDDR4_TYPE */
		}
	}
}

#if STM32MP_LPDDR4_TYPE
/*
 * Programming Seq0BGPR1/2/3 for LPDDR4
 */
static void seq0bgpr_program(struct stm32mp_ddr_config *config)
{
	uint32_t extradly = 3U;
	uint32_t rl = 0U; /* Computed read latency */
	uint32_t wl = 0U; /* Computed write latency */
	uint16_t mr_dbi_rd; /* Extracted field from MR */
	uint16_t mr_rl;
	uint16_t mr_wl;
	uint16_t mr_wls;
	uint16_t regdata;

	mr_rl = (uint16_t)config->uia.lp4rl;	/* RL[2:0] */
	mr_wl = (uint16_t)config->uia.lp4wl;	/* WL[5:3] */
	mr_wls = (uint16_t)config->uia.lp4wls;	/* WLS */
	mr_dbi_rd = (uint16_t)config->uia.lp4dbird; /* DBI-RD */

	switch ((mr_dbi_rd << 3) | mr_rl) {
		/* DBI-RD Disabled */
	case  0U:
		rl = 6U;
		break;
	case  1U:
		rl = 10U;
		break;
	case  2U:
		rl = 14U;
		break;
	case  3U:
		rl = 20U;
		break;
	case  4U:
		rl = 24U;
		break;
	case  5U:
		rl = 28U;
		break;
	case  6U:
		rl = 32U;
		break;
	case  7U:
		rl = 36U;
		break;
		/* DBI-RD Enabled */
	case  8U:
		rl = 6U;
		break;
	case  9U:
		rl = 12U;
		break;
	case 10U:
		rl = 16U;
		break;
	case 11U:
		rl = 22U;
		break;
	case 12U:
		rl = 28U;
		break;
	case 13U:
		rl = 32U;
		break;
	case 14U:
		rl = 36U;
		break;
	case 15U:
		rl = 40U;
		break;
	default:
		rl = 6U;
		break;
	}

	switch ((mr_wls << 3) | mr_wl) {
		/* DBI-RD Disabled */
	case  0U:
		wl = 4U;
		break;
	case  1U:
		wl = 6U;
		break;
	case  2U:
		wl = 8U;
		break;
	case  3U:
		wl = 10U;
		break;
	case  4U:
		wl = 12U;
		break;
	case  5U:
		wl = 14U;
		break;
	case  6U:
		wl = 16U;
		break;
	case  7U:
		wl = 18U;
		break;
		/* DBI-RD Enabled */
	case  8U:
		wl = 4U;
		break;
	case  9U:
		wl = 8U;
		break;
	case 10U:
		wl = 12U;
		break;
	case 11U:
		wl = 18U;
		break;
	case 12U:
		wl = 22U;
		break;
	case 13U:
		wl = 26U;
		break;
	case 14U:
		wl = 30U;
		break;
	case 15U:
		wl = 34U;
		break;
	default:
		wl = 4U;
		break;
	}

	/* Program Seq0b_GPRx */
	regdata = (uint16_t)((rl - 5U + extradly) << CSR_ACSMRCASLAT_LSB);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (P0 | C0 | TINITENG | R2 |
							CSR_SEQ0BGPR1_ADDR))),
		      regdata);

	regdata = (uint16_t)((wl - 5U + extradly) << CSR_ACSMWCASLAT_LSB);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (P0 | C0 | TINITENG | R2 |
							CSR_SEQ0BGPR2_ADDR))),
		      regdata);

	regdata = (uint16_t)((rl - 5U + extradly + 4U + 8U) << CSR_ACSMRCASLAT_LSB);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (P0 | C0 | TINITENG | R2 |
							CSR_SEQ0BGPR3_ADDR))),
		      regdata);
}

/*
 * Program hwtlpcsena and hwtlpcsenb based on number of ranks per channel
 * Applicable only for LPDDR4.  These CSRs have no effect for DDR3/4.
 *
 * CSRs to program:
 *      hwtlpcsena
 *      hwtlpcsenb
 *
 * User input dependencies:
 *      config->uib.numrank_dfi0
 *      config->uib.numrank_dfi1
 *      config->uib.dfi1exists
 *      config->uib.numactivedbytedfi1
 */
static void hwtlpcsen_program(struct stm32mp_ddr_config *config)
{
	uint16_t hwtlpcsena;
	uint16_t hwtlpcsenb;

	/* Channel A - 1'b01 if signal-rank, 2'b11 if dual-rank */
	hwtlpcsena = (uint16_t)config->uib.numrank_dfi0 | 0x1U;
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTLPCSENA_ADDR))),
		      hwtlpcsena);

	/*
	 * Channel B - 1'b01 if signal-rank, 2'b11 if dual-rank
	 * If DFI1 exists but disabled, numrank_dfi0 is used to program CsEnB
	 */
	if ((config->uib.dfi1exists != 0U) && (config->uib.numactivedbytedfi1 == 0U)) {
		hwtlpcsenb = (uint16_t)config->uib.numrank_dfi0 | 0x1U;
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTLPCSENB_ADDR))),
			      hwtlpcsenb);
	} else if ((config->uib.dfi1exists != 0U) && (config->uib.numactivedbytedfi1 > 0U)) {
		hwtlpcsenb = (uint16_t)config->uib.numrank_dfi1 | 0x1U;
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTLPCSENB_ADDR))),
			      hwtlpcsenb);
	} else {
		/* Disable Channel B */
		hwtlpcsenb = 0x0U;
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTLPCSENB_ADDR))),
			      hwtlpcsenb);
	}
}

/*
 * Program pptdqscntinvtrntg0 and pptdqscntinvtrntg1
 * Calculated based on tDQS2DQ and Frequencey
 * Applicable to LPDDR4 only
 *
 * 65536*(tdqs2dq_value_rank<rank>_chan<chan>*2)/(2*2048*UI(ps)_int)
 *
 * CSRs to program:
 *      pptdqscntinvtrntg0
 *      pptdqscntinvtrntg1
 *
 * User input dependencies:
 *      config->uib.numrank_dfi0
 *      config->uib.numrank_dfi1
 *      config->uib.dfi1exists
 *      config->uib.numdbyte
 */
static void pptdqscntinvtrntg_program(struct stm32mp_ddr_config *config)
{
	uint32_t numrank_total = config->uib.numrank_dfi0;
	uint32_t rank;

	/* Calculate total number of timing groups (ranks) */
	if (config->uib.dfi1exists != 0U) {
		numrank_total += config->uib.numrank_dfi1;
	}

	/* Set per timing group */
	for (rank = 0U; rank < numrank_total; rank++) {
		uint32_t byte;

		for (byte = 0U; byte < config->uib.numdbyte; byte++) {
			uint32_t c_addr;

			c_addr = byte << 12;
			if (rank == 0U) {
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr |
							       CSR_PPTDQSCNTINVTRNTG0_ADDR))),
					      0U);
			} else if (rank == 1U) {
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (TDBYTE | c_addr |
							       CSR_PPTDQSCNTINVTRNTG1_ADDR))),
					      0U);
			}
		}
	}
}

/*
 * CSRs to program:
 *      PptCtlStatic:: DOCByteSelTg0/1
 *                   :: pptenrxenbackoff
 *
 * User input dependencies::
 *      config->uib.numdbyte
 *      config->uib.numrank_dfi0
 *      config->uib.numrank_dfi1
 *      config->uia.lp4rxpreamblemode
 *      config->uia.rxenbackoff
 *      config->uia.drambyteswap
 */
static void pptctlstatic_program(struct stm32mp_ddr_config *config)
{
	uint32_t byte;
	uint32_t pptenrxenbackoff;

	/*
	 * The customer will setup some fields in this csr so the fw needs to do a
	 * read-modify-write here.
	 */

	if (config->uia.lp4rxpreamblemode == 1U) {
		/* Rx-preamble mode for PS0 */
		/* Programming PptCtlStatic detected toggling preamble */
		pptenrxenbackoff = 0x1U; /* Toggling RD_PRE */
	} else {
		pptenrxenbackoff = config->uia.rxenbackoff; /* Static RD_PRE */
	}

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint16_t regdata;
		uint8_t pptentg1;
		uint32_t docbytetg0;
		uint32_t docbytetg1;

		/* Each Dbyte could have a different configuration */
		c_addr = byte * C1;
		if ((byte % 2) == 0) {
			docbytetg0 = 0x1U & (config->uia.drambyteswap >> byte);
			docbytetg1 = 0x1U & (config->uia.drambyteswap >> byte);
		} else {
			docbytetg0 = 0x1U & (~(config->uia.drambyteswap >> byte));
			docbytetg1 = 0x1U & (~(config->uia.drambyteswap >> byte));
		}

		pptentg1 = ((config->uib.numrank_dfi0 == 2U) || (config->uib.numrank_dfi1 == 2U)) ?
			   0x1U : 0x0U;
		regdata = (uint16_t)((0x1U << CSR_PPTENDQS2DQTG0_LSB) |
				     (pptentg1 << CSR_PPTENDQS2DQTG1_LSB) |
				     (0x1U << CSR_PPTENRXENDLYTG0_LSB) |
				     (pptentg1 << CSR_PPTENRXENDLYTG1_LSB) |
				     (pptenrxenbackoff << CSR_PPTENRXENBACKOFF_LSB) |
				     (docbytetg0 << CSR_DOCBYTESELTG0_LSB) |
				     (docbytetg1 << CSR_DOCBYTESELTG1_LSB));

		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								CSR_PPTCTLSTATIC_ADDR))),
			      regdata);
	}
}
#endif /* STM32MP_LPDDR4_TYPE */

/*
 * Program hwtcamode based on dram type
 *
 * CSRs to program:
 *      hwtcamode::hwtlp3camode
 *               ::hwtd4camode
 *               ::hwtlp4camode
 *               ::hwtd4altcamode
 *               ::hwtcsinvert
 *               ::hwtdbiinvert
 */
static void hwtcamode_program(void)
{
	uint32_t hwtlp3camode = 0U;
	uint32_t hwtd4camode = 0U;
	uint32_t hwtlp4camode = 0U;
	uint32_t hwtd4altcamode = 0U;
	uint32_t hwtcsinvert = 0U;
	uint32_t hwtdbiinvert = 0U;
	uint16_t hwtcamode;

#if STM32MP_DDR4_TYPE
	hwtd4camode = 1U;
#elif STM32MP_LPDDR4_TYPE
	hwtlp4camode = 1U;
	hwtcsinvert = 1U;
	hwtdbiinvert = 1U;
#else /* STM32MP_DDR3_TYPE */
	/* Nothing to declare */
#endif /* STM32MP_DDR4_TYPE */

	hwtcamode = (uint16_t)((hwtdbiinvert << CSR_HWTDBIINVERT_LSB) |
			       (hwtcsinvert << CSR_HWTCSINVERT_LSB) |
			       (hwtd4altcamode << CSR_HWTD4ALTCAMODE_LSB) |
			       (hwtlp4camode << CSR_HWTLP4CAMODE_LSB) |
			       (hwtd4camode << CSR_HWTD4CAMODE_LSB) |
			       (hwtlp3camode << CSR_HWTLP3CAMODE_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_HWTCAMODE_ADDR))), hwtcamode);
}

/*
 * Program DllGainCtl and DllLockParam based on frequency
 */
static void dllgainctl_dlllockparam_program(struct stm32mp_ddr_config *config)
{
	uint32_t dllgainiv;
	uint32_t dllgaintv;
	uint32_t lcdlseed;
	uint32_t memck_freq;
	uint32_t stepsize_x10 = 47U;	/*
					 * Nominal stepsize, in units of tenths of a ps,
					 * if nominal=4.7ps use 47
					 */
	uint16_t wddllgainctl;
	uint16_t wddlllockparam;

	memck_freq = config->uib.frequency;

	/*
	 * lcdlseed = ((1000000/memck_freq)/2)/lcdl_stepsize  ...
	 * where default lcdl_stepsize=4.7 in simulation.
	 */
	if (memck_freq >= 1200U) {
		dllgainiv = 0x04U;
		dllgaintv = 0x05U;
	} else if (memck_freq >= 800U) {
		dllgainiv = 0x03U;
		dllgaintv = 0x05U;
	} else if (memck_freq >= 532U) {
		dllgainiv = 0x02U;
		dllgaintv = 0x04U;
	} else if (memck_freq >= 332U) {
		dllgainiv = 0x01U;
		dllgaintv = 0x03U;
	} else {
		dllgainiv = 0x00U;
		dllgaintv = 0x02U;
	}

	/*
	 * lcdlseed= (1000000/(2*memck_freq)) * (100/(120*(stepsize_nominal)));
	 * *100/105 is to bias the seed low.
	 */
	lcdlseed = (1000000U * 10U * 100U) / (2U * memck_freq * stepsize_x10 * 105U);

	if (lcdlseed > (511U - 32U)) {
		lcdlseed = 511U - 32U;
	}

	if (lcdlseed < 32U) {
		lcdlseed = 32U;
	}

	wddllgainctl = (uint16_t)((CSR_DLLGAINTV_MASK & (dllgaintv << CSR_DLLGAINTV_LSB)) |
				  (CSR_DLLGAINIV_MASK & (dllgainiv << CSR_DLLGAINIV_LSB)));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DLLGAINCTL_ADDR))),
		      wddllgainctl);

	wddlllockparam = (uint16_t)((CSR_LCDLSEED0_MASK & (lcdlseed << CSR_LCDLSEED0_LSB)) |
				    (CSR_DISDLLGAINIVSEED_MASK & 0xFFFFU));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DLLLOCKPARAM_ADDR))),
		      wddlllockparam);
}

/*
 * Program AcsmCtrl23 for Fw and Ppt.
 *
 * CSRs to program:
 *   AcsmCtrl23::AcsmCsMask
 *               AcsmCsMode
 */
static void acsmctrl23_program(void)
{
	uint16_t regdata;

	regdata = (0x0FU << CSR_ACSMCSMASK_LSB) | (0x1U << CSR_ACSMCSMODE_LSB);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TACSM | CSR_ACSMCTRL23_ADDR))),
		      regdata);
}

/*
 * Set PllForceCal to 1 and PllDacValIn to some arbitrary value
 */
static void pllforcecal_plldacvalin_program(void)
{
	uint32_t dacval_in = 0x10U;
	uint32_t force_cal = 0x1U;
	uint32_t pllencal = 0x1U;
	uint32_t maxrange = 0x1FU;
	uint16_t pllctrl3_gpr;
	uint16_t pllctrl3_startup;

	pllctrl3_startup = (uint16_t)((dacval_in << CSR_PLLDACVALIN_LSB) |
				      (maxrange << CSR_PLLMAXRANGE_LSB));
	pllctrl3_gpr = pllctrl3_startup | (uint16_t)((force_cal << CSR_PLLFORCECAL_LSB) |
						     (pllencal << CSR_PLLENCAL_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_PLLCTRL3_ADDR))),
		      pllctrl3_startup);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BGPR6_ADDR))),
		      pllctrl3_gpr);
}

/*
 * This function programs registers that are normally set by training
 * firmware.
 *
 * This function is used in place of running 1D or 1D training steps. PhyInit
 * calls this function when skip_train = true. In that case, PhyInit does not
 * execute training firmware and this function is called instead to program
 * PHY registers according to DRAM timing parameters specified in userInput
 * data structure. See documentation of ddrphy_phyinit_struct.h file
 * details of timing parameters available in skip training.
 *
 * \warning ddrphy_phyinit_progcsrskiptrain() only supports zero board
 * delay model. If system board delays are set or randomized, full 1D or 1D
 * initialization flow must be executed.
 *
 * This function replaces these steps in the PHY Initialization sequence:
 *  - (E) Set the PHY input clocks to the desired frequency
 *  - (F) Write the Message Block parameters for the training firmware
 *  - (G) Execute the Training Firmware
 *  - (H) Read the Message Block results
 *
 * \returns \c void
 */
void ddrphy_phyinit_progcsrskiptrain(struct stm32mp_ddr_config *config,
				     struct pmu_smb_ddr_1d *mb_ddr_1d, uint32_t ardptrinitval)
{
	uint16_t txdqsdly;

	/*
	 * Program ATxDlY
	 * For DDR4, DDR3 and LPDDR4, leave AtxDly[6:0] at default (0x0)
	 */

	dfimrl_program(config, mb_ddr_1d, ardptrinitval);

	txdqsdlytg_program(config, mb_ddr_1d, &txdqsdly);

	txdqdlytg_program(config, mb_ddr_1d, txdqsdly);

	rxendly_program(config, mb_ddr_1d);

#if STM32MP_LPDDR4_TYPE
	seq0bgpr_program(config);

	hwtlpcsen_program(config);

	pptdqscntinvtrntg_program(config);

	pptctlstatic_program(config);
#endif /* STM32MP_LPDDR4_TYPE */

	hwtcamode_program();

	dllgainctl_dlllockparam_program(config);

	acsmctrl23_program();

	pllforcecal_plldacvalin_program();

	/*
	 * ##############################################################
	 *
	 * Setting PhyInLP3 to 0 to cause PIE to execute LP2 sequence instead of INIT on first
	 * dfi_init_start.
	 * This prevents any DRAM commands before DRAM is initialized, which is the case for
	 * skip_train.
	 *
	 * Moved to here from dddrphy_phyinit_I_loadPIEImage()
	 * These should not be needed on S3-exit
	 *
	 * Note this executes for SkipTrain only, *not* DevInit+SkipTrain
	 * DevInit+SkipTrain already initializes DRAM and thus don't need to avoid DRAM commands
	 *
	 * ##############################################################
	 */

	/*
	 * Special skipTraining configuration to Prevent DRAM Commands on the first dfi
	 * status interface handshake. In order to see this behavior, the first dfi_freq
	 * should be in the range of 0x0f < dfi_freq_sel[4:0] < 0x14.
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_PHYINLP3_ADDR))), 0x0U);
}
