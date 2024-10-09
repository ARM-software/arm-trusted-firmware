/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>

/*
 * This is used to initialize the PhyInit structures before user defaults and overrides are applied.
 *
 * @return Void
 */
void ddrphy_phyinit_initstruct(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d)
{
	/*
	 * ##############################################################
	 * Basic Message Block Variables
	 * ##############################################################
	 */

	uint8_t msgmisc = 0x00U;	/* For fast simulation */
	uint8_t reserved00 = 0x0U;	/*
					 * Set reserved00[7] = 1 (If using T28 attenuated receivers)
					 * Set reserved00[6:0] = 0 (Reserved; must be set to 0)
					 */

	uint8_t hdtctrl = 0xFFU;
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	uint8_t cspresent = 0x01U;	/*
					 * Indicates presence of DRAM at each chip select for PHY.
					 *
					 * If the bit is set to 1, the CS is connected to DRAM.
					 * If the bit is set to 0, the CS is not connected to DRAM.
					 *
					 * Set cspresent[0]   = 1 (if CS0 is populated with DRAM)
					 * Set cspresent[1]   = 1 (if CS1 is populated with DRAM)
					 * Set cspresent[2]   = 1 (if CS2 is populated with DRAM)
					 * Set cspresent[3]   = 1 (if CS3 is populated with DRAM)
					 * Set cspresent[7:4] = 0 (Reserved; must be set to 0)
					 */
	uint8_t dfimrlmargin = 0x01U;	/* 1 is typically good in DDR3 */
#if STM32MP_DDR3_TYPE
	uint8_t addrmirror = 0x00U;	/*
					 * Set addrmirror if CS is mirrored.
					 * (typically odd CS are mirroed in DIMMs)
					 */
#else /* STM32MP_DDR4_TYPE */
	uint8_t addrmirror = 0xAAU;
#endif /* STM32MP_DDR3_TYPE */
	uint8_t wrodtpat_rank0 = 0x01U;	/*
					 * When Writing Rank0 : Bits[3:0] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
	uint8_t wrodtpat_rank1 = 0x02U;	/*
					 * When Writing Rank1 : Bits[3:0] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
#if STM32MP_DDR3_TYPE
	uint8_t wrodtpat_rank2 = 0x04U;	/*
					 * When Writing Rank2 : Bits[3:0] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
	uint8_t wrodtpat_rank3 = 0x08U;	/*
					 * When Writing Rank3 : Bits[3:0] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
#else /* STM32MP_DDR4_TYPE */
	uint8_t wrodtpat_rank2 = 0x00U;
	uint8_t wrodtpat_rank3 = 0x00U;
#endif /* STM32MP_DDR3_TYPE */
	uint8_t rdodtpat_rank0 = 0x20U;	/*
					 * When Reading Rank0 : Bits[7:4] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
	uint8_t rdodtpat_rank1 = 0x10U;	/*
					 * When Reading Rank1 : Bits[7:4] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
#if STM32MP_DDR3_TYPE
	uint8_t rdodtpat_rank2 = 0x80U;	/*
					 * When Reading Rank2 : Bits[7:4] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
	uint8_t rdodtpat_rank3 = 0x40U;	/*
					 * When Reading Rank3 : Bits[7:4] should be set to the
					 * desired setting of ODT[3:0] to the DRAM
					 */
#else /* STM32MP_DDR4_TYPE */
	uint8_t rdodtpat_rank2 = 0x00U;
	uint8_t rdodtpat_rank3 = 0x00U;

	uint8_t d4misc = 0x1U;		/*
					 * Protect memory reset:
					 * 0x1 = dfi_reset_n cannot control BP_MEMRESERT_L to
					 *       devices after training.
					 * 0x0 = dfi_resert_n can control BP_MEMRESERT_L to
					 *       devices after training.
					 */
#endif /* STM32MP_DDR3_TYPE */
#else /* STM32MP_LPDDR4_TYPE */
	uint8_t caterminatingrankcha = 0x00U; /* Usually Rank0 is terminating rank */
	uint8_t caterminatingrankchb = 0x00U; /* Usually Rank0 is terminating rank */
	uint8_t dfimrlmargin = 0x02U; /* This needs to be large enough for max tDQSCK variation */
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

#if STM32MP_DDR3_TYPE
	uint8_t share2dvrefresult = 0x0U;	/*
						 * Bitmap that controls which vref generator the
						 * phy will use per pstate
						 *     If share2dvrefresult[x] = 1, pstate x will
						 *     use the per-lane VrefDAC0/1 CSRs which can be
						 *     trained by 2d training. If 2D has not run
						 *     yet, VrefDAC0/1 will default to pstate 0's
						 *     1D phyVref messageBlock setting.
						 *     If share2dvrefresult[x] = 0, pstate x will
						 *     use the per-phy VrefInGlobal CSR, which are
						 *     set to pstate x's 1D phyVref messageBlock
						 *     setting.
						 */
#elif STM32MP_DDR4_TYPE
	uint8_t share2dvrefresult = 0x1U;
#else /* STM32MP_LPDDR4_TYPE */
	uint8_t share2dvrefresult = 0x1U;
	uint8_t usebroadcastmr = 0x00U;
#endif /* STM32MP_DDR3_TYPE */

	/* 1D message block defaults */
	memset((void *)mb_ddr_1d, 0, sizeof(struct pmu_smb_ddr_1d));

	mb_ddr_1d->pstate = 0U;
	mb_ddr_1d->sequencectrl = (uint16_t)config->uia.sequencectrl;
	mb_ddr_1d->phyconfigoverride = 0x0U;
	mb_ddr_1d->hdtctrl = hdtctrl;
	mb_ddr_1d->msgmisc = msgmisc;
	mb_ddr_1d->reserved00 = reserved00;
	mb_ddr_1d->dfimrlmargin = dfimrlmargin;
	mb_ddr_1d->phyvref = (uint8_t)config->uia.phyvref;

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	mb_ddr_1d->cspresent = cspresent;
	mb_ddr_1d->cspresentd0 = cspresent;
	/* mb_ddr_1d->cspresentd1 = 0x0U; Unused */
	mb_ddr_1d->addrmirror = addrmirror;

	mb_ddr_1d->acsmodtctrl0 = wrodtpat_rank0 | rdodtpat_rank0;
	mb_ddr_1d->acsmodtctrl1 = wrodtpat_rank1 | rdodtpat_rank1;
	mb_ddr_1d->acsmodtctrl2 = wrodtpat_rank2 | rdodtpat_rank2;
	mb_ddr_1d->acsmodtctrl3 = wrodtpat_rank3 | rdodtpat_rank3;

	/* mb_ddr_1d->acsmodtctrl4 = 0x0U; Unused */
	/* mb_ddr_1d->acsmodtctrl5 = 0x0U; Unused */
	/* mb_ddr_1d->acsmodtctrl6 = 0x0U; Unused */
	/* mb_ddr_1d->acsmodtctrl7 = 0x0U; Unused */
	mb_ddr_1d->enableddqs = (uint8_t)((config->uib.numactivedbytedfi0 +
					   config->uib.numactivedbytedfi1) * 8U);
#if STM32MP_DDR3_TYPE
	mb_ddr_1d->phycfg = (uint8_t)config->uia.is2ttiming;
#else /* STM32MP_DDR4_TYPE */
	mb_ddr_1d->phycfg = ((config->uim.mr3 & 0x8U) == 0x8U) ?
			      0U : (uint8_t)config->uia.is2ttiming;
	mb_ddr_1d->x16present = (config->uib.dramdatawidth == 0x10) ?
				mb_ddr_1d->cspresent : 0x0U;
	mb_ddr_1d->d4misc = d4misc;
	mb_ddr_1d->cssetupgddec = 0x1U;	/* If Geardown is chosen, dynamically modify CS timing */

	/*
	 * Outputs - just initialize these to zero
	 * mb_ddr_1d->rtt_nom_wr_park<0..7>
	 */
#endif /* STM32MP_DDR3_TYPE */

	mb_ddr_1d->mr0 = (uint16_t)config->uim.mr0;
	mb_ddr_1d->mr1 = (uint16_t)config->uim.mr1;
	mb_ddr_1d->mr2 = (uint16_t)config->uim.mr2;
#if STM32MP_DDR4_TYPE
	mb_ddr_1d->mr3 = (uint16_t)config->uim.mr3;
	mb_ddr_1d->mr4 = (uint16_t)config->uim.mr4;
	mb_ddr_1d->mr5 = (uint16_t)config->uim.mr5;
	mb_ddr_1d->mr6 = (uint16_t)config->uim.mr6;

	mb_ddr_1d->alt_cas_l = 0x0U;
	mb_ddr_1d->alt_wcas_l = 0x0U;

	/*
	 * Outputs - just initialize these to zero
	 * mb_ddr_1d->vrefdqr<0..3>nib<0..19>
	 */
#endif /* STM32MP_DDR4_TYPE */
#else /* STM32MP_LPDDR4_TYPE */
	mb_ddr_1d->enableddqscha = (uint8_t)(config->uib.numactivedbytedfi0 * 8U);
	mb_ddr_1d->cspresentcha = (config->uib.numrank_dfi0 == 2U) ?
				  0x3U : (uint8_t)config->uib.numrank_dfi0;
	mb_ddr_1d->enableddqschb = (uint8_t)(config->uib.numactivedbytedfi1 * 8U);
	mb_ddr_1d->cspresentchb = (config->uib.numrank_dfi1 == 2U) ?
				  0x3U : (uint8_t)config->uib.numrank_dfi1;
	mb_ddr_1d->usebroadcastmr = usebroadcastmr;

	mb_ddr_1d->lp4misc = 0x00U;
	mb_ddr_1d->caterminatingrankcha = caterminatingrankcha;
	mb_ddr_1d->caterminatingrankchb = caterminatingrankchb;
	mb_ddr_1d->lp4quickboot = 0x00U;
	mb_ddr_1d->catrainopt = 0x00U;
	mb_ddr_1d->x8mode = 0x00U;

	mb_ddr_1d->mr1_a0 = (uint8_t)config->uim.mr1;
	mb_ddr_1d->mr2_a0 = (uint8_t)config->uim.mr2;
	mb_ddr_1d->mr3_a0 = (uint8_t)config->uim.mr3;
	mb_ddr_1d->mr4_a0 = (uint8_t)config->uim.mr4;
	mb_ddr_1d->mr11_a0 = (uint8_t)config->uim.mr11;
	mb_ddr_1d->mr12_a0 = (uint8_t)config->uim.mr12;
	mb_ddr_1d->mr13_a0 = (uint8_t)config->uim.mr13;
	mb_ddr_1d->mr14_a0 = (uint8_t)config->uim.mr14;
	mb_ddr_1d->mr16_a0 = 0x00U;
	mb_ddr_1d->mr17_a0 = 0x00U;
	mb_ddr_1d->mr22_a0 = (uint8_t)config->uim.mr22;
	mb_ddr_1d->mr24_a0 = 0x00U;
	mb_ddr_1d->mr1_a1 = (uint8_t)config->uim.mr1;
	mb_ddr_1d->mr2_a1 = (uint8_t)config->uim.mr2;
	mb_ddr_1d->mr3_a1 = (uint8_t)config->uim.mr3;
	mb_ddr_1d->mr4_a1 = (uint8_t)config->uim.mr4;
	mb_ddr_1d->mr11_a1 = (uint8_t)config->uim.mr11;
	mb_ddr_1d->mr12_a1 = (uint8_t)config->uim.mr12;
	mb_ddr_1d->mr13_a1 = (uint8_t)config->uim.mr13;
	mb_ddr_1d->mr14_a1 = (uint8_t)config->uim.mr14;
	mb_ddr_1d->mr16_a1 = 0x00U;
	mb_ddr_1d->mr17_a1 = 0x00U;
	mb_ddr_1d->mr22_a1 = (uint8_t)config->uim.mr22;
	mb_ddr_1d->mr24_a1 = 0x00U;

	mb_ddr_1d->mr1_b0 = (uint8_t)config->uim.mr1;
	mb_ddr_1d->mr2_b0 = (uint8_t)config->uim.mr2;
	mb_ddr_1d->mr3_b0 = (uint8_t)config->uim.mr3;
	mb_ddr_1d->mr4_b0 = (uint8_t)config->uim.mr4;
	mb_ddr_1d->mr11_b0 = (uint8_t)config->uim.mr11;
	mb_ddr_1d->mr12_b0 = (uint8_t)config->uim.mr12;
	mb_ddr_1d->mr13_b0 = (uint8_t)config->uim.mr13;
	mb_ddr_1d->mr14_b0 = (uint8_t)config->uim.mr14;
	mb_ddr_1d->mr16_b0 = 0x00U;
	mb_ddr_1d->mr17_b0 = 0x00U;
	mb_ddr_1d->mr22_b0 = (uint8_t)config->uim.mr22;
	mb_ddr_1d->mr24_b0 = 0x00U;
	mb_ddr_1d->mr1_b1 = (uint8_t)config->uim.mr1;
	mb_ddr_1d->mr2_b1 = (uint8_t)config->uim.mr2;
	mb_ddr_1d->mr3_b1 = (uint8_t)config->uim.mr3;
	mb_ddr_1d->mr4_b1 = (uint8_t)config->uim.mr4;
	mb_ddr_1d->mr11_b1 = (uint8_t)config->uim.mr11;
	mb_ddr_1d->mr12_b1 = (uint8_t)config->uim.mr12;
	mb_ddr_1d->mr13_b1 = (uint8_t)config->uim.mr13;
	mb_ddr_1d->mr14_b1 = (uint8_t)config->uim.mr14;
	mb_ddr_1d->mr16_b1 = 0x00U;
	mb_ddr_1d->mr17_b1 = 0x00U;
	mb_ddr_1d->mr22_b1 = (uint8_t)config->uim.mr22;
	mb_ddr_1d->mr24_b1 = 0x00U;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	mb_ddr_1d->share2dvrefresult = share2dvrefresult;
}
