/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>
#include <ddrphy_wrapper.h>

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/*
 * Program txslewrate:
 * - txslewrate::txpredrvmode is dependent on dramtype.
 * - txslewrate::txprep and txslewrate::txpren are technology-specific.
 */
static void txslewrate_program(struct stm32mp_ddr_config *config)
{
	uint32_t txpredrvmode;
	uint32_t byte;
	uint32_t txpren; /* Default to 0xf (max). Optimal setting is technology specific */
	uint32_t txprep; /* Default to 0xf (max). Optimal setting is technology specific */
	uint16_t txslewrate;

#if STM32MP_DDR3_TYPE
	txpredrvmode = 0x3U;
#elif STM32MP_DDR4_TYPE
	txpredrvmode = 0x2U;
#else /* STM32MP_LPDDR4_TYPE */
	txpredrvmode = 0x1U;
#endif /* STM32MP_DDR3_TYPE */

	txprep = config->uia.txslewrisedq;
	txpren = config->uia.txslewfalldq;

	txslewrate = (uint16_t)((txpredrvmode << CSR_TXPREDRVMODE_LSB) |
				(txpren << CSR_TXPREN_LSB) |
				(txprep << CSR_TXPREP_LSB));

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t lane;

		c_addr = byte << 12;
		for (lane = 0U; lane <= B_MAX; lane++) {
			uint32_t b_addr;

			b_addr = lane << 8;
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDBYTE | c_addr | b_addr |
									CSR_TXSLEWRATE_ADDR))),
				      txslewrate);
		}
	}
}

/*
 * Program atxslewrate:
 * - atxslewrate::atxpredrvmode is dependent on dramtype and whether
 *   the ACX4 instance is used for AC or CK.
 * - atxslewrate::atxprep and atxslewrate::atxpren are technology-specific.
 */
static void atxslewrate_program(struct stm32mp_ddr_config *config)
{
	uint32_t anib;
	uint32_t atxpren; /* Default to 0xf (max). Optimal setting is technology specific */
	uint32_t atxprep; /* Default to 0xf (max). Optimal setting is technology specific */
	uint32_t ck_anib_inst[2] = {0};

	atxprep = config->uia.txslewriseac;
	atxpren = config->uia.txslewfallac;

	/*
	 * # of ANIBs      CK ANIB Instance
	 * ACX8            ANIB 1
	 */
	if (config->uib.numanib == 8U) {
		ck_anib_inst[0] = 1U;
		ck_anib_inst[1] = 1U;
	}

	for (anib = 0U; anib < config->uib.numanib; anib++) {
		uint32_t atxpredrvmode;
		uint32_t c_addr;
		uint16_t atxslewrate;

		c_addr = anib << 12;

		if ((anib == ck_anib_inst[0]) || (anib == ck_anib_inst[1])) {
			/* CK ANIB instance */
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
			atxpredrvmode = 0x0U;
#else /* STM32MP_LPDDR4_TYPE */
			atxpredrvmode = 0x1U;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
		} else {
			/* non-CK ANIB instance */
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
			atxpredrvmode = 0x3U;
#else /* STM32MP_LPDDR4_TYPE */
			atxpredrvmode = 0x1U;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
		}

		atxslewrate = (uint16_t)((atxpredrvmode << CSR_ATXPREDRVMODE_LSB) |
					 (atxpren << CSR_ATXPREN_LSB) |
					 (atxprep << CSR_ATXPREP_LSB));

		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TANIB | c_addr |
								CSR_ATXSLEWRATE_ADDR))),
			      atxslewrate);
	}
}

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
/*
 * Program dfirddatacsdestmap and dfiwrdatacsdestmap:
 * - Dependencies: mb_ddr_1d->msgmisc[6] Determine Partial Rank Support.
 */
static void dfidatacsdestmap_program(struct pmu_smb_ddr_1d *mb_ddr_1d)
{
	if ((mb_ddr_1d->msgmisc & 0x40U) != 0U) {
		uint16_t dfirddatacsdestmap = 0xA0U;
		uint16_t dfiwrdatacsdestmap = 0xA0U;

		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER |
								CSR_DFIRDDATACSDESTMAP_ADDR))),
			      dfirddatacsdestmap);
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER |
								CSR_DFIWRDATACSDESTMAP_ADDR))),
			      dfiwrdatacsdestmap);
	}
}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

/*
 * Program pllctrl2:
 * - Calculate PLL controls from frequency.
 */
static void pllctrl2_program(struct stm32mp_ddr_config *config)
{
	uint16_t pllctrl2;
	uint32_t halffreq = config->uib.frequency / 2U;

	if (halffreq < 235U) {
		pllctrl2 = 0x7U;
	} else if (halffreq < 313U) {
		pllctrl2 = 0x6U;
	} else if (halffreq < 469U) {
		pllctrl2 = 0xBU;
	} else if (halffreq < 625U) {
		pllctrl2 = 0xAU;
	} else if (halffreq < 938U) {
		pllctrl2 = 0x19U;
	} else if (halffreq < 1067U) {
		pllctrl2 = 0x18U;
	} else {
		pllctrl2 = 0x19U;
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_PLLCTRL2_ADDR))), pllctrl2);
}

/*
 * Program ardptrinitval:
 * - The values programmed here assume ideal properties of DfiClk and Pclk including:
 *   - DfiClk skew
 *   - DfiClk jitter
 *   - DfiClk PVT variations
 *   - Pclk skew
 *   - Pclk jitter
 *
 * ardptrinitval Programmed differently based on PLL Bypass mode and frequency:
 * - PLL Bypassed mode:
 *   - For MemClk frequency > 933MHz, the valid range of ardptrinitval[3:0] is: 2-5
 *   - For MemClk frequency < 933MHz, the valid range of ardptrinitval[3:0] is: 1-5
 * - PLL Enabled mode:
 *   - For MemClk frequency > 933MHz, the valid range of ardptrinitval[3:0] is: 1-5
 *   - For MemClk frequency < 933MHz, the valid range of ardptrinitval[3:0] is: 0-5
 */
static void ardptrinitval_program(struct stm32mp_ddr_config *config, uint32_t *ardptrinitval)
{
	uint16_t regdata;

	if (config->uib.frequency >= 933U) {
		regdata = 0x2U;
	} else {
		regdata = 0x1U;
	}

	/* Add one UI for synchronizer on SyncBus when PLL is bypassed */
	if (config->uib.pllbypass == 1U) {
		regdata++;
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_ARDPTRINITVAL_ADDR))),
		      regdata);

	*ardptrinitval = (uint32_t)regdata;
}

#if STM32MP_LPDDR4_TYPE
/*
 * Program ProcOdtCtl:
 * - Sets procodtalwayson/procodtalwaysoff for LPDDR4 using the PIE register seq0bgpr4.
 */
static void procodtctl_program(void)
{
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | C0 | CSR_SEQ0BGPR4_ADDR))), 0U);
}
#endif /* STM32MP_LPDDR4_TYPE */

/*
 * Program dbytedllmodecntrl:
 * - dllrxpreamblemode
 * Program dqspreamblecontrol:
 * - Fields:
 *   - twotckrxdqspre
 *   - twotcktxdqspre
 *   - positiondfeinit
 *   - lp4tgltwotcktxdqspre
 *   - lp4postambleext
 *   - lp4sttcprebridgerxen
 * - Dependencies:
 *   - user_input_advanced.lp4rxpreamblemode (LPDDR4)
 *   - user_input_advanced.lp4postambleext (LPDDR4)
 *   - user_input_advanced.wdqsext (LPDDR4)
 *   - user_input_advanced.d4rxpreamblelength (DDR4)
 *   - user_input_advanced.d4txpreamblelength (DDR4)
 */
static void dbytedllmodecntrl_program(struct stm32mp_ddr_config *config, uint32_t *twotckrxdqspre)
{
	uint32_t disdllgainivseed = 1U;
	uint32_t disdllseedsel = 0U;
	uint32_t dllgainiv = 0x1U;
	uint32_t dllgaintv = 0x6U;
	uint32_t dllrxpreamblemode = 0U;
	uint32_t lcdlseed0 = 0x21U;
	uint32_t lp4postambleext = 0U;
	uint32_t lp4sttcprebridgerxen = 0U;
	uint32_t lp4tgltwotcktxdqspre = 0U;
	uint32_t positiondfeinit;
	uint32_t twotcktxdqspre = 0U;
	uint32_t wdqsextension = 0U;
	uint16_t dbytedllmodecntrl;
	uint16_t dllgainctl;
	uint16_t dlllockparam;
	uint16_t dqspreamblecontrol;

#if STM32MP_DDR3_TYPE
	/* Same as default */
	*twotckrxdqspre		= 0x0U;
	lp4sttcprebridgerxen	= 0x0U;
	dllrxpreamblemode	= 0x0U;
	twotcktxdqspre		= 0x0U;
	lp4tgltwotcktxdqspre	= 0x0U;
	positiondfeinit		= 0x0U;
	lp4postambleext		= 0x0U;
#elif STM32MP_DDR4_TYPE
	*twotckrxdqspre		= config->uia.d4rxpreamblelength;
	lp4sttcprebridgerxen	= 0x0U;
	dllrxpreamblemode	= 0x1U;
	twotcktxdqspre		= config->uia.d4txpreamblelength;
	lp4tgltwotcktxdqspre	= 0x0U;
	positiondfeinit		= 0x2U;
	lp4postambleext		= 0x0U;
#else /* STM32MP_LPDDR4_TYPE */
	/* Set to 1 if static Rx preamble */
	*twotckrxdqspre		= (config->uia.lp4rxpreamblemode == 0U) ? 1U : 0U;
	/* Set to 1 if static Rx preamble */
	lp4sttcprebridgerxen	= (config->uia.lp4rxpreamblemode == 0U) ? 1U : 0U;
	dllrxpreamblemode	= 0x1U;
	/* Must be 2*Tck Tx preamble according to JEDEC (mr1.OP[2] = 1) */
	twotcktxdqspre		= 0x1U;
	/* Must be toggling Tx preamble */
	lp4tgltwotcktxdqspre	= 0x1U;
	positiondfeinit		= 0x0U;
	lp4postambleext		= config->uia.lp4postambleext;
	wdqsextension		= config->uia.wdqsext;
#endif /* STM32MP_DDR3_TYPE */

	dqspreamblecontrol = (uint16_t)((wdqsextension << CSR_WDQSEXTENSION_LSB) |
					(lp4sttcprebridgerxen << CSR_LP4STTCPREBRIDGERXEN_LSB) |
					(lp4postambleext << CSR_LP4POSTAMBLEEXT_LSB) |
					(lp4tgltwotcktxdqspre << CSR_LP4TGLTWOTCKTXDQSPRE_LSB) |
					(positiondfeinit << CSR_POSITIONDFEINIT_LSB) |
					(twotcktxdqspre << CSR_TWOTCKTXDQSPRE_LSB) |
					(*twotckrxdqspre << CSR_TWOTCKRXDQSPRE_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DQSPREAMBLECONTROL_ADDR))),
		      dqspreamblecontrol);

	dbytedllmodecntrl = (uint16_t)(dllrxpreamblemode << CSR_DLLRXPREAMBLEMODE_LSB);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DBYTEDLLMODECNTRL_ADDR))),
		      dbytedllmodecntrl);

	dllgainctl = (uint16_t)(dllgainiv | (dllgaintv << CSR_DLLGAINTV_LSB));
	dlllockparam = (uint16_t)(disdllseedsel | (disdllgainivseed << CSR_DISDLLGAINIVSEED_LSB) |
				  (lcdlseed0 << CSR_LCDLSEED0_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DLLLOCKPARAM_ADDR))),
		      dlllockparam);

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DLLGAINCTL_ADDR))),
		      dllgainctl);
}

/*
 * Program procodttimectl:
 * - Fields:
 *   - POdtStartDelay[3:2]
 *   - POdtTailWidth[1:0]
 * - Dependencies:
 *   - user_input_basic.frequency
 *   - user_input_advanced.wdqsext
 */
static void procodttimectl_program(struct stm32mp_ddr_config *config, uint32_t twotckrxdqspre)
{
	uint16_t procodttimectl;

	if (config->uia.wdqsext != 0U) {
		/* POdtStartDelay = 0x0 and  POdtTailWidth  = 0x3 */
		procodttimectl = 0x3U;
	} else if (config->uib.frequency <= 933U) {
		/* Memclk Freq <= 933MHz: POdtStartDelay = 0x2 and POdtTailWidth  = 0x2 */
		procodttimectl = 0xAU;
	} else if (config->uib.frequency <= 1200U) {
		/* 933MHz < Memclk Freq <= 1200MHz */
		if (twotckrxdqspre == 1U) {
			/* POdtStartDelay = 0x0 and  POdtTailWidth  = 0x2 */
			procodttimectl = 0x2U;
		} else {
			/* POdtStartDelay = 0x1 and POdtTailWidth  = 0x2 */
			procodttimectl = 0x6U;
		}
	} else {
		/* Memclk Freq > 1200MHz */
		if (twotckrxdqspre == 1U) {
			/* POdtStartDelay = 0x0 and POdtTailWidth  = 0x3 */
			procodttimectl = 0x3U;
		} else {
			/* POdtStartDelay = 0x1 and POdtTailWidth  = 0x3 */
			procodttimectl = 0x7U;
		}
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_PROCODTTIMECTL_ADDR))),
		      procodttimectl);
}

/*
 * Program txodtdrvstren:
 * - Fields:
 *   - ODTStrenP_px[5:0]
 *   - ODTStrenN_px[11:6]
 * - Dependencies:
 *   - user_input_basic.numdbyte
 *   - user_input_advanced.odtimpedance
 * \return 0 on success.
 */
static int txodtdrvstren_program(struct stm32mp_ddr_config *config)
{
	uint32_t byte;
	int odtstrenn_state;
	int odtstrenp_state;
	uint16_t txodtdrvstren;

	odtstrenp_state = ddrphy_phyinit_mapdrvstren(config->uia.odtimpedance, ODTSTRENP);
	if (odtstrenp_state < 0) {
		return odtstrenp_state;
	}

	odtstrenn_state = ddrphy_phyinit_mapdrvstren(config->uia.odtimpedance, ODTSTRENN);
	if (odtstrenn_state < 0) {
		return odtstrenn_state;
	}

	txodtdrvstren = (uint16_t)((odtstrenn_state << CSR_ODTSTRENN_LSB) | odtstrenp_state);

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t lane;

		c_addr = byte << 12;
		for (lane = 0U; lane <= B_MAX; lane++) {
			uint32_t b_addr;

			b_addr = lane << 8;
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDBYTE | c_addr | b_addr |
									CSR_TXODTDRVSTREN_ADDR))),
				      txodtdrvstren);
		}
	}

	return 0;
}

/*
 * Program tximpedancectrl1:
 * - Fields:
 *   - DrvStrenFSDqP[5:0]
 *   - DrvStrenFSDqN[11:6]
 * - Dependencies:
 *   - user_input_basic.numdbyte
 *   - user_input_advanced.tximpedance
 * \return 0 on success.
 */
static int tximpedancectrl1_program(struct stm32mp_ddr_config *config)
{
	uint32_t byte;
	int drvstrenfsdqn_state;
	int drvstrenfsdqp_state;
	uint16_t tximpedancectrl1;

	drvstrenfsdqp_state = ddrphy_phyinit_mapdrvstren(config->uia.tximpedance,
							 DRVSTRENFSDQP);
	if (drvstrenfsdqp_state < 0) {
		return drvstrenfsdqp_state;
	}

	drvstrenfsdqn_state = ddrphy_phyinit_mapdrvstren(config->uia.tximpedance,
							 DRVSTRENFSDQN);
	if (drvstrenfsdqn_state < 0) {
		return drvstrenfsdqn_state;
	}

	tximpedancectrl1 = (uint16_t)((drvstrenfsdqn_state << CSR_DRVSTRENFSDQN_LSB) |
				      (drvstrenfsdqp_state << CSR_DRVSTRENFSDQP_LSB));

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t lane;

		c_addr = byte << 12;
		for (lane = 0U; lane <= B_MAX; lane++) {
			uint32_t b_addr;

			b_addr = lane << 8;
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U *
								  (TDBYTE | c_addr | b_addr |
								   CSR_TXIMPEDANCECTRL1_ADDR))),
				      tximpedancectrl1);
		}
	}

	return 0;
}

/*
 * Program atximpedance:
 * - Fields:
 *   - ADrvStrenP[4:0]
 *   - ADrvStrenN[9:5]
 * - Dependencies:
 *   - user_input_basic.numanib
 *   - user_input_advanced.atximpedance
 * \return 0 on success.
 */
static int atximpedance_program(struct stm32mp_ddr_config *config)
{
	int adrvstrenn_state;
	int adrvstrenp_state;
	uint32_t anib;
	uint16_t atximpedance;

	adrvstrenp_state = ddrphy_phyinit_mapdrvstren(config->uia.atximpedance,
						      ADRVSTRENP);
	if (adrvstrenp_state < 0) {
		return adrvstrenp_state;
	}

	adrvstrenn_state = ddrphy_phyinit_mapdrvstren(config->uia.atximpedance,
						      ADRVSTRENN);
	if (adrvstrenn_state < 0) {
		return adrvstrenn_state;
	}

	atximpedance = (uint16_t)((adrvstrenn_state << CSR_ADRVSTRENN_LSB) |
				  (adrvstrenp_state << CSR_ADRVSTRENP_LSB));

	for (anib = 0U; anib < config->uib.numanib; anib++) {
		uint32_t c_addr;

		c_addr = anib << 12;
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TANIB | c_addr |
								CSR_ATXIMPEDANCE_ADDR))),
			      atximpedance);
	}

	return 0;
}

/*
 * Program dfimode:
 * - Dependencies:
 *   - user_input_basic.dfi1exists
 */
static void dfimode_program(struct stm32mp_ddr_config *config)
{
	uint16_t dfimode = 0x5U;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	if (config->uib.dfi1exists == 0U) {
		dfimode = 0x1U; /* DFI1 does not physically exists */
	}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DFIMODE_ADDR))), dfimode);
}

/*
 * Program dficamode:
 * - Fields:
 *   - DfiLp3CAMode
 *   - DfiD4CAMode
 *   - DfiLp4CAMode
 *   - DfiD4AltCAMode
 */
static void dficamode_program(void)
{
	uint16_t dficamode;

#if STM32MP_DDR3_TYPE
	dficamode = 0U;
#elif STM32MP_DDR4_TYPE
	dficamode = 2U;
#else /* STM32MP_LPDDR4_TYPE */
	dficamode = 4U;
#endif /* STM32MP_DDR3_TYPE */

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DFICAMODE_ADDR))), dficamode);
}

/*
 * Program caldrvstr0:
 * - Fields:
 *   - caldrvstrpd50[3:0]
 *   - caldrvstrpu50[7:4]
 * - Dependencies:
 *   - user_input_advanced.extcalresval
 */
static void caldrvstr0_program(struct stm32mp_ddr_config *config)
{
	uint16_t caldrvstr0;
	uint16_t caldrvstrp50 = (uint16_t)config->uia.extcalresval;

	caldrvstr0 = (caldrvstrp50 << CSR_CALDRVSTRPU50_LSB) | caldrvstrp50;

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_CALDRVSTR0_ADDR))),
		      caldrvstr0);
}

/*
 * Program CalUclkInfo:
 * - Impedance calibration CLK Counter.
 * - Fields:
 *   - caluclkticksper1us
 * - Dependencies:
 *   - user_input_basic.frequency
 */
static void caluclkinfo_program(struct stm32mp_ddr_config *config)
{
	uint32_t caluclkticksper1us_x10;
	uint16_t caluclkticksper1us;

	/* Number of DfiClk cycles per 1us */
	caluclkticksper1us_x10 = (10U * config->uib.frequency) / 2U;
	caluclkticksper1us = (uint16_t)(caluclkticksper1us_x10 / 10U);

	if ((config->uib.frequency % 2U) != 0U) {
		caluclkticksper1us++;
	}

	if (caluclkticksper1us < 24U) {
		/* Minimum value of caluclkticksper1us = 24 */
		caluclkticksper1us = 24U;
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_CALUCLKINFO_ADDR))),
		      caluclkticksper1us);
}

/*
 * Program Calibration CSRs based on user input
 * - Fields:
 *   - calinterval
 *   - calonce
 * - Dependencies:
 *   - user_input_advanced.calinterval
 *   - user_input_advanced.calonce
 */
static void calibration_program(struct stm32mp_ddr_config *config)
{
	uint32_t calinterval;
	uint32_t calonce;
	uint16_t calrate;

	calinterval = config->uia.calinterval;
	calonce = config->uia.calonce;

	calrate = (uint16_t)((calonce << CSR_CALONCE_LSB) | (calinterval << CSR_CALINTERVAL_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_CALRATE_ADDR))), calrate);
}

/*
 * Program vrefinglobal:
 * - dqdqsrcvcntrl and csrvrefinglobal to select Global VREF
 *   from Master to be used in each DQ.
 * - Fields:
 *   - globalvrefinsel: Select Range of GlobalVref DAC. Default: set to 1.
 *   - globalvrefindac: Vref level is set based on mb_ddr_1d->phyvref value.
 *     The following formula is used to convert the phyvref into the register setting.
 *       \f{eqnarray*}{
 *           PhyVrefPrcnt &=& \frac{mb_ddr_1d->phyvref}{128} \\
 *        if globalvrefinsel = 1 :
 *           globalvrefindac &=& 1+\frac{PhyVrefPrcnt}{0.005} \\
 *        if globalvrefinsel = 0 :
 *           globalvrefindac &=& \frac{(PhyVrefPrcnt-0.345)}{0.005} \\
 *           RxVref &=& (globalvrefindac == 0) ? Hi-Z : (PhyVrefPrcnt \times VDDQ)
 *        \f}
 *
 * Program dqdqsrcvcntrl:
 * - dqdqsrcvcntrl and csrvrefinglobal to select Global VREF
 *   from Master to be used in each DQ
 * - Fields:
 *  - selanalogvref
 *  - majormodedbyte
 *  - ExtVrefRange
 *  - DfeCtrl
 *  - GainCurrAdj
 * - Dependencies:
 *   - user_input_basic.numdbyte
 */
static void vrefinglobal_program(struct stm32mp_ddr_config *config,
				 struct pmu_smb_ddr_1d *mb_ddr_1d)
{
	uint32_t majormodedbyte;
	int32_t vref_percentvddq = (int32_t)mb_ddr_1d->phyvref * 1000 * 100 / 128;
	uint8_t globalvrefindac = 0x0U;
	uint8_t globalvrefinsel = 0x4U;
	uint32_t byte;
	uint32_t dfectrl_defval = 0U;
	uint32_t extvrefrange_defval = 0U;
	uint32_t gaincurradj_defval = 0xBU;
	uint32_t selanalogvref = 1U; /* Use Global VREF from Master */
	uint16_t dqdqsrcvcntrl;
	uint16_t vrefinglobal;

#if STM32MP_DDR3_TYPE
	majormodedbyte = 0U;
#elif STM32MP_DDR4_TYPE
	majormodedbyte = 3U;
#else /* STM32MP_LPDDR4_TYPE */
	majormodedbyte = 2U;
#endif /* STM32MP_DDR3_TYPE */

	/* Check range1 first. Only use range0 if customer input maxes out range1. */
	globalvrefindac = (uint8_t)((vref_percentvddq / 500) + 1);
	if (globalvrefindac > 127U) {
		/* Min value is 1 */
		globalvrefindac = (uint8_t)(MAX((vref_percentvddq - 34500), 500) / 500);
		globalvrefinsel = 0x0U;
	}
	globalvrefindac = MIN(globalvrefindac, (uint8_t)127);

	vrefinglobal = (uint16_t)((globalvrefindac << CSR_GLOBALVREFINDAC_LSB) | globalvrefinsel);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_VREFINGLOBAL_ADDR))),
		      vrefinglobal);

	dqdqsrcvcntrl = (uint16_t)((gaincurradj_defval << CSR_GAINCURRADJ_LSB) |
				   (majormodedbyte << CSR_MAJORMODEDBYTE_LSB) |
				   (dfectrl_defval << CSR_DFECTRL_LSB) |
				   (extvrefrange_defval << CSR_EXTVREFRANGE_LSB) |
				   (selanalogvref << CSR_SELANALOGVREF_LSB));

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t lane;

		c_addr = byte << 12;
		for (lane = 0U; lane <= B_MAX; lane++) {
			uint32_t b_addr;

			b_addr = lane << 8;
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDBYTE | c_addr | b_addr |
									CSR_DQDQSRCVCNTRL_ADDR))),
				      dqdqsrcvcntrl);
		}
	}
}

/*
 * Program dfifreqratio :
 * - Dependencies:
 *   - user_input_basic.dfifreqratio
 */
static void dfifreqratio_program(struct stm32mp_ddr_config *config)
{
	uint16_t dfifreqratio;

	dfifreqratio = (uint16_t)config->uib.dfifreqratio;

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DFIFREQRATIO_ADDR))),
		      dfifreqratio);
}

/*
 * Program tristatemodeca based on dramtype and 2T Timing
 * - Fields:
 *   - CkDisVal
 *   - disdynadrtri
 *   - ddr2tmode
 * - Dependencies:
 *   - user_input_advanced.is2ttiming
 *   - user_input_advanced.disdynadrtri
 */
static void tristatemodeca_program(struct stm32mp_ddr_config *config)
{
	uint32_t ckdisval_def;
	uint32_t ddr2tmode;
	uint32_t disdynadrtri;
	uint16_t tristatemodeca;

	/* CkDisVal depends on dramtype */
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	ckdisval_def = 1U; /* {CLK_t,CLK_c} = 2'b00; */
#else /* STM32MP_LPDDR4_TYPE */
	ckdisval_def = 0U; /* {CLK_t,CLK_c} = 2'b01; */
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	disdynadrtri = config->uia.disdynadrtri;
#else /* STM32MP_LPDDR4_TYPE */
	disdynadrtri = 1U;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	ddr2tmode = config->uia.is2ttiming;

	tristatemodeca = (uint16_t)((ckdisval_def << CSR_CKDISVAL_LSB) |
				    (ddr2tmode << CSR_DDR2TMODE_LSB) |
				    (disdynadrtri << CSR_DISDYNADRTRI_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_TRISTATEMODECA_ADDR))),
		      tristatemodeca);
}

/*
 * Program DfiXlat based on Pll Bypass Input
 * - Dependencies:
 *   - user_input_basic.frequency
 *   - user_input_basic.pllbypass
 */
static void dfixlat_program(struct stm32mp_ddr_config *config)
{
	uint16_t loopvector;
	uint16_t pllbypass_dat = 0U;

	pllbypass_dat |= (uint16_t)config->uib.pllbypass;

	for (loopvector = 0U; loopvector < 8U; loopvector++) {
		uint16_t dfifreqxlat_dat;
		uintptr_t reg = (uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TMASTER |
								  (CSR_DFIFREQXLAT0_ADDR +
								   loopvector))));

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
		if (loopvector == 0U) {
			/*
			 * Relock DfiFreq = 00,01,02,03)  Use StartVec 5 (pll_enabled) or
			 * StartVec 6 (pll_bypassed).
			 */
			dfifreqxlat_dat = pllbypass_dat + 0x5555U;

			mmio_write_16(reg, dfifreqxlat_dat);
		} else if (loopvector == 7U) {
			/* LP3-entry DfiFreq = 1F */
			mmio_write_16(reg, 0xF000U);
		} else {
			/*
			 * Everything else = skip retrain  (could also map to 0000 since retrain
			 * code is excluded, but this is cleaner).
			 */
			mmio_write_16(reg, 0x5555U);
		}
#else /* STM32MP_LPDDR4_TYPE */
		if (loopvector == 0U) {
			uint16_t skipddc_dat = 0U;	/*
							 * Set to vector offset based on frequency
							 * to disable dram drift compensation.
							 */

			if (config->uib.frequency < 333U) {
				skipddc_dat |= 0x5U;
			}

			/*
			 * Retrain & Relock DfiFreq = 00,01,02,03)  Use StartVec 0 (pll_enabled) or
			 * StartVec 1 (pll_bypassed).
			 */
			dfifreqxlat_dat = pllbypass_dat + skipddc_dat;
			mmio_write_16(reg, dfifreqxlat_dat);
		} else if (loopvector == 2U) {
			/*
			 * Retrain only DfiFreq = 08,09,0A,0B)  Use StartVec 4 (1, and maybe 2,3,
			 * used by verif).
			 */
			mmio_write_16(reg, 0x4444U);
		} else if (loopvector == 3U) {
			/* Phymstr type state change, StartVec 8 */
			mmio_write_16(reg, 0x8888U);
		} else if (loopvector == 4U) {
			/*
			 * Relock only DfiFreq = 10,11,12,13   Use StartVec 5 (pll_enabled) or
			 * StartVec 6 (pll_bypassed).
			 */
			dfifreqxlat_dat = pllbypass_dat + 0x5555U;
			mmio_write_16(reg, dfifreqxlat_dat);
		} else if (loopvector == 7U) {
			/* LP3-entry DfiFreq = 1F */
			mmio_write_16(reg, 0xF000U);
		} else {
			/* Everything else */
			mmio_write_16(reg, 0x0000U);
		}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
	}
}

/*
 * Program dqdqsrcvcntrl1 (Receiver Powerdown) and DbyteMiscMode
 * - see function ddrphy_phyinit_isdbytedisabled() to determine
 *   which DBytes are turned off completely based on PHY configuration.
 * - Fields:
 *   - DByteDisable
 *   - PowerDownRcvr
 *   - PowerDownRcvrDqs
 *   - RxPadStandbyEn
 * - Dependencies:
 *   - user_input_basic.numdbyte
 *   - user_input_basic.dramdatawidth (DDR3/DDR4)
 *   - mb_ddr_1d->mr5 (DDR4)
 *   - user_input_advanced.lp4dbird (LPDDR4)
 */
static void dqdqsrcvcntrl1_program(struct stm32mp_ddr_config *config,
				   struct pmu_smb_ddr_1d *mb_ddr_1d)
{
	uint32_t d;
	uint16_t mr5 __maybe_unused;
	uint16_t regdata;
	uint16_t regdata1;
	uint16_t regdata2; /* Turn off Rx of DBI lane */

	regdata = 0x1U << CSR_DBYTEDISABLE_LSB;

	regdata1 = (0x1FFU << CSR_POWERDOWNRCVR_LSB) |
		   (0x1U << CSR_POWERDOWNRCVRDQS_LSB) |
		   (0x1U << CSR_RXPADSTANDBYEN_LSB);

	regdata2 = (0x100U << CSR_POWERDOWNRCVR_LSB) | CSR_RXPADSTANDBYEN_MASK;

#if STM32MP_DDR4_TYPE
	/* OR all mr4 masked values, to help check in next loop */
	mr5 = (mb_ddr_1d->mr5 >> 12) & 0x1U;
#endif /* STM32MP_DDR4_TYPE */

	for (d = 0U; d < config->uib.numdbyte; d++) {
		uint32_t c_addr;

		c_addr = d * C1;
		if (ddrphy_phyinit_isdbytedisabled(config, mb_ddr_1d, d) != 0) {
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
									CSR_DBYTEMISCMODE_ADDR))),
				      regdata);
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
									CSR_DQDQSRCVCNTRL1_ADDR))),
				      regdata1);
		} else {
			/* Disable RDBI lane if not used. */
#if STM32MP_DDR3_TYPE
			if (config->uib.dramdatawidth != 4U) {
#elif STM32MP_DDR4_TYPE
			if ((config->uib.dramdatawidth != 4U) && (mr5 == 0U)) {
#else /* STM32MP_LPDDR4_TYPE */
			if (config->uia.lp4dbird == 0U) {
#endif /* STM32MP_DDR3_TYPE */
				mmio_write_16((uintptr_t)
					      (DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								     CSR_DQDQSRCVCNTRL1_ADDR))),
					      regdata2);
			}
		}
	}
}

/*
 * Program masterx4config
 * - Fields:
 *   - x4tg
 *   - masterx4config
 * - Dependencies:
 *   - user_input_basic.dramdatawidth
 *
 * \note PHY does not support mixed dram device data width
 */
static void masterx4config_program(struct stm32mp_ddr_config *config)
{
	uint32_t x4tg = 0U;
	uint16_t masterx4config;

	if (config->uib.dramdatawidth == 4U) {
		x4tg = 0xFU;
	}

	masterx4config = (uint16_t)(x4tg << CSR_X4TG_LSB);

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_MASTERX4CONFIG_ADDR))),
		      masterx4config);
}

#if !STM32MP_DDR3_TYPE
/*
 * Program dmipinpresent based on dramtype and Read-DBI enable
 * - Fields:
 *   - RdDbiEnabled
 * - Dependencies:
 *   - mb_ddr_1d->mr5 (DDR4)
 *   - user_input_advanced.lp4dbird (LPDDR4)
 */
static void dmipinpresent_program(struct stm32mp_ddr_config *config,
				  struct pmu_smb_ddr_1d *mb_ddr_1d)
{
	uint16_t dmipinpresent;

#if STM32MP_DDR4_TYPE
	/* For DDR4, Read DBI is enabled in mr5-A12 */
	dmipinpresent = (mb_ddr_1d->mr5 >> 12) & 0x1U;
#else /* STM32MP_LPDDR4_TYPE */
	dmipinpresent = (uint16_t)config->uia.lp4dbird;
#endif /* STM32MP_DDR4_TYPE */

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DMIPINPRESENT_ADDR))),
		      dmipinpresent);
}
#endif /* !STM32MP_DDR3_TYPE */

/*
 * Program aforcetricont and acx4anibdis
 * - Fields:
 *   - aforcetricont
 *   - acx4anibdis
 * - Dependencies:
 *   - user_input_basic.numrank_dfi0
 *   - user_input_basic.numrank_dfi1
 *   - user_input_basic.numanib
 *   - user_input_advanced.disableunusedaddrlns
 */
static void aforcetricont_acx4anibdis_program(struct stm32mp_ddr_config *config)
{
	uint32_t anib;
	uint16_t acx4anibdis = 0x0U;

	for (anib = 0U; (anib < config->uib.numanib) && (config->uia.disableunusedaddrlns != 0U);
	     anib++) {
		uint32_t c_addr;
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
		uint32_t numrank = config->uib.numrank_dfi0 + config->uib.numrank_dfi1;
#else /* STM32MP_LPDDR4_TYPE */
		uint32_t numrank0 = config->uib.numrank_dfi0;
		uint32_t numrank1 = config->uib.numrank_dfi1;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
		uint16_t aforcetricont = 0x0U;

		c_addr = anib << 12;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
		if ((anib == 0U) && (numrank == 1U)) {
			aforcetricont = 0x2U;
		} else if ((anib == 1U) && (numrank == 1U)) {
			aforcetricont = 0xCU;
		} else if (anib == 6U) {
			aforcetricont = 0x1U;
		}
#else /* STM32MP_LPDDR4_TYPE */
		if ((anib == 0U) && (numrank0 == 0U)) {
			aforcetricont = 0xFU;
		} else if ((anib == 0U) && (numrank0 == 1U)) {
			aforcetricont = 0x2U;
		} else if ((anib == 1U) && (numrank0 == 0U)) {
			aforcetricont = 0xFU;
		} else if ((anib == 1U) && (numrank0 == 1U)) {
			aforcetricont = 0x8U;
		} else if ((anib == 2U) && (numrank0 == 0U)) {
			aforcetricont = 0xFU;
		} else if ((anib == 3U) && (numrank1 == 0U)) {
			aforcetricont = 0xFU;
		} else if ((anib == 3U) && (numrank1 == 1U)) {
			aforcetricont = 0x2U;
		} else if ((anib == 4U) && (numrank1 == 0U)) {
			aforcetricont = 0xFU;
		} else if ((anib == 4U) && (numrank1 == 1U)) {
			aforcetricont = 0x8U;
		} else if ((anib == 5U) && (numrank1 == 0U)) {
			aforcetricont = 0xFU;
		} else if (anib == 6U) {
			aforcetricont = 0xFU;
		} else if (anib == 7U) {
			aforcetricont = 0xFU;
		}

		/*
		 * If all the lanes can be disabled, and Anib is not the first or last disable
		 * entire chiplet
		 */
		if ((aforcetricont == 0xFU) && (anib != 0U) &&
		    (anib != (config->uib.numanib - 1U))) {
			acx4anibdis = acx4anibdis | (0x1U << anib);
		}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TANIB | c_addr |
								CSR_AFORCETRICONT_ADDR))),
			      aforcetricont);
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_ACX4ANIBDIS_ADDR))),
		      acx4anibdis);
}

/*
 * Implements Step C of initialization sequence
 *
 * This function programs majority of PHY configuration registers based
 * on data input into PhyInit data structures.
 *
 * This function programs PHY configuration registers based on information
 * provided in the PhyInit data structures (config->uib, config->uia).
 * The user can overwrite the programming of this function by modifying
 * ddrphy_phyinit_usercustom_custompretrain().  Please see
 * ddrphy_phyinit_struct.h for PhyInit data structure definition.
 *
 * \return 0 on success.
 */
int ddrphy_phyinit_c_initphyconfig(struct stm32mp_ddr_config *config,
				   struct pmu_smb_ddr_1d *mb_ddr_1d, uint32_t *ardptrinitval)
{
	uint32_t twotckrxdqspre;
	int ret;

	/*
	 * Step (C) Initialize PHY Configuration
	 * Load the required PHY configuration registers for the appropriate mode and memory
	 * configuration.
	 */

	txslewrate_program(config);

	atxslewrate_program(config);

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	dfidatacsdestmap_program(mb_ddr_1d);
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	pllctrl2_program(config);

	ardptrinitval_program(config, ardptrinitval);

#if STM32MP_LPDDR4_TYPE
	procodtctl_program();
#endif /* STM32MP_LPDDR4_TYPE */

	dbytedllmodecntrl_program(config, &twotckrxdqspre);

	procodttimectl_program(config, twotckrxdqspre);

	ret = txodtdrvstren_program(config);
	if (ret != 0) {
		return ret;
	}

	ret = tximpedancectrl1_program(config);
	if (ret != 0) {
		return ret;
	}

	ret = atximpedance_program(config);
	if (ret != 0) {
		return ret;
	}

	dfimode_program(config);

	dficamode_program();

	caldrvstr0_program(config);

	caluclkinfo_program(config);

	calibration_program(config);

	vrefinglobal_program(config, mb_ddr_1d);

	dfifreqratio_program(config);

	tristatemodeca_program(config);

	dfixlat_program(config);

	dqdqsrcvcntrl1_program(config, mb_ddr_1d);

	masterx4config_program(config);

#if !STM32MP_DDR3_TYPE
	dmipinpresent_program(config, mb_ddr_1d);

#if STM32MP_LPDDR4_TYPE
	/*
	 * Program DFIPHYUPD
	 * - Fields:
	 *   - DFIPHYUPDMODE
	 *   - DFIPHYUPDCNT
	 * - Dependencies:
	 *   - user_input_advanced.disablephyupdate
	 */
	if (config->uia.disablephyupdate != 0U) {
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_DFIPHYUPD_ADDR))),
			      0x0U);
	}
#endif /* STM32MP_LPDDR4_TYPE */
#endif /* !STM32MP_DDR3_TYPE */

	aforcetricont_acx4anibdis_program(config);

	return 0;
}
