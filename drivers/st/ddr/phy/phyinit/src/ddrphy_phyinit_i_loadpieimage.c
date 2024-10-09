/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>

#include <lib/mmio.h>

#include <platform_def.h>

#if STM32MP_LPDDR4_TYPE
/*
 * Program DfiWrRdDataCsConfig
 * - Fields:
 *   - dfiwrdatacspolarity
 *   - dfirddatacspolarity
 */
static void dfiwrrddatacsconfig_program(void)
{
	uint16_t dfiwrdatacspolarity;
	uint16_t dfirddatacspolarity;

	/*
	 * DfiWrRdDataCsConfig : dfiwrdatacspolarity=0x1 and dfirddatacspolarity=0x1.
	 * Set DataCsPolarity bits to enable active high
	 */
	dfiwrdatacspolarity = 0x1U;
	dfirddatacspolarity = 0x1U;

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | C0 |
							CSR_DFIWRRDDATACSCONFIG_ADDR))),
		      (dfiwrdatacspolarity << CSR_DFIWRDATACSPOLARITY_LSB) |
		      (dfirddatacspolarity << CSR_DFIRDDATACSPOLARITY_LSB));
}
#endif /* STM32MP_LPDDR4_TYPE */

/*
 * Registers: Seq0BDLY0, Seq0BDLY1, Seq0BDLY2, Seq0BDLY3
 * - Program PIE instruction delays
 * - Dependencies:
 *   - user_input_basic.frequency
 */
static void seq0bdly_program(struct stm32mp_ddr_config *config)
{
	uint16_t lowfreqopt __unused;
	uint16_t dfifrq_x10;
	uint16_t pscount_ref;
	uint16_t pscount[4]; /* Need delays for 0.5us, 1us, 10us, and 25us */

	/*
	 * Calculate the counts to obtain the correct delay for each frequency
	 * Need to divide by 4 since the delay value are specified in units of
	 * 4 clocks.
	 */
	dfifrq_x10 = (10U * (uint16_t)config->uib.frequency) / 2U;
	pscount_ref = dfifrq_x10 / 4U;
	pscount[0] = pscount_ref / (2U * 10U);

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	if (config->uib.frequency < 400U) {
		lowfreqopt = 3U;
	} else if (config->uib.frequency < 533U) {
		lowfreqopt = 11U;
	} else {
		lowfreqopt = 0U;
	}

	pscount[1] = (pscount_ref / 10U) - lowfreqopt;
#else /* STM32MP_LPDDR4_TYPE */
	pscount[1] = pscount_ref / 10U;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
	pscount[2] = pscount_ref;

	if (dfifrq_x10 > 2665U) {
		pscount[3] = 44U;
	} else if ((dfifrq_x10 <= 2665U) && (dfifrq_x10 > 2000U)) {
		pscount[3] = 33U;
	} else {
		pscount[3] = 16U;
	}

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TMASTER | CSR_SEQ0BDLY0_ADDR))),
		      pscount[0]);

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TMASTER | CSR_SEQ0BDLY1_ADDR))),
		      pscount[1]);

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TMASTER | CSR_SEQ0BDLY2_ADDR))),
		      pscount[2]);

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TMASTER | CSR_SEQ0BDLY3_ADDR))),
		      pscount[3]);
}

/*
 * Registers: Seq0BDisableFlag0..7
 * - Program PIE Instruction Disable Flags
 * - Dependencies:
 *   - user_input_advanced.DisableRetraining (LPDDR4)
 *   - skip_training (LPDDR4)
 *   - user_input_basic.frequency (LPDDR4)
 */
static void seq0bdisableflag_program(struct stm32mp_ddr_config *config, bool skip_training)
{
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG0_ADDR))),
		      0x0000U);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG1_ADDR))),
		      0x0173U);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG2_ADDR))),
		      0x0060U);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG3_ADDR))),
		      0x6110U);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG4_ADDR))),
		      0x2152U);
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG5_ADDR))),
		      0xDFBDU);

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG6_ADDR))),
		      0xFFFFU);
#else /* STM32MP_LPDDR4_TYPE */
	if (skip_training || (config->uia.disableretraining != 0U) ||
	    (config->uib.frequency < 333U)) {
		/* Disabling DRAM drift compensation */
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG |
								CSR_SEQ0BDISABLEFLAG6_ADDR))),
			      0xFFFFU);
	} else {
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG |
								CSR_SEQ0BDISABLEFLAG6_ADDR))),
			      0x2060U);
	}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	/*
	 * - Register: Seq0BGPR7
	 *   - Program active CSx for MRS7 during D4 RDIMM frequency change
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TINITENG | CSR_SEQ0BDISABLEFLAG7_ADDR))),
		      0x6152U);
}

#if STM32MP_LPDDR4_TYPE
/*
 * Registers: ppttrainsetup and ppttrainsetup2
 * - Related to DFI PHY Master Interface (PMI).
 * - Enable DFI PMI if training firmware was run
 * - Fields:
 *   - PhyMstrTrainInterval
 *   - PhyMstrMaxReqToAck
 *   - PhyMstrFreqOverride
 * - Dependencies:
 *   - user_input_basic.frequency
 *   - user_input_advanced.PhyMstrTrainInterval
 *   - user_input_advanced.PhyMstrMaxReqToAck
 */
static void ppttrainsetup_program(struct stm32mp_ddr_config *config)
{
	uint16_t ppttrainsetup;

	/* Enabling Phy Master Interface for DRAM drift compensation */
	if (config->uib.frequency >= 333U) {
		ppttrainsetup =	(uint16_t)((config->uia.phymstrtraininterval <<
					    CSR_PHYMSTRTRAININTERVAL_LSB) |
					   (config->uia.phymstrmaxreqtoack <<
					    CSR_PHYMSTRMAXREQTOACK_LSB));

		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_PPTTRAINSETUP_ADDR))),
			      ppttrainsetup);
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER |
								CSR_PPTTRAINSETUP2_ADDR))),
			      0x0003U);
	}
}

/*
 * Registers AcsmPlayback*x*
 * - Program Address/Command Sequence Engine (ACSM) registers with
 *   required instructions for retraining algorithm.
 */
static void acsmplayback_program(void)
{
	uint32_t vec;

	for (vec = 0U; vec < 3U; vec++) {
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TACSM | (CSR_ACSMPLAYBACK0X0_ADDR +
									 (vec * 2U))))),
			      0xE0U);

		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TACSM | (CSR_ACSMPLAYBACK1X0_ADDR +
									 (vec * 2U))))),
			      0x12U);
	}
}

/*
 * Program Training Hardware Registers for mission mode retraining
 * and DRAM drift compensation algorithm.
 */
static void traininghwreg_program(struct stm32mp_ddr_config *config)
{
	uint32_t byte;

	/* Programing Training Hardware Registers for mission mode retraining */

	/*
	 * - Register: AcsmCtrl13
	 *   - Fields: AcsmCkeEnb
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TACSM | CSR_ACSMCTRL13_ADDR))),
		      0xFU << CSR_ACSMCKEENB_LSB);

	/*
	 * - Register: AcsmCtrl1
	 *   - Fields: AcsmRepCnt
	 *             Need 19 iterations @ 0.25ui increments to cover 4.5UI
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (C0 | TACSM | CSR_ACSMCTRL1_ADDR))),
		      0xEU << CSR_ACSMREPCNT_LSB);

	/*
	 * - Register: TsmByte1, TsmByte2
	 *   - Dependencies: config->uib.numdbyte
	 */

	for (byte = 0U; byte < config->uib.numdbyte; byte++) {
		uint32_t c_addr;
		uint32_t i_addr;
		uint16_t regdata;
		uint32_t vec;

		c_addr = byte * C1;
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								CSR_TSMBYTE1_ADDR))),
			      0x1U);   /* [15:8] gstep; [7:0]bstep; */
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								CSR_TSMBYTE2_ADDR))),
			      0x1U);   /* [15:0] good_bar; */

		regdata = (CSR_DTSMSTATICCMPR_MASK | CSR_DTSMSTATICCMPRVAL_MASK);

		/*
		 * - Register: TsmByte3, TsmByte5
		 *   - Fields:
		 *     - DtsmStaticCmpr
		 *     - DtsmStaticCmprVal
		 */
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								CSR_TSMBYTE3_ADDR))),
			      regdata);
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								CSR_TSMBYTE5_ADDR))),
			      0x1U); /* [15:0] bad_bar; */

		/*
		 * - Register: TrainingParam
		 *   - Fields:
		 *     - EnDynRateReduction
		 *     - RollIntoCoarse
		 *     - IncDecRate
		 *     - TrainEnRxEn
		 *   - Dependencies:
		 *     - user_input_advanced.DisableRetraining
		 */
		regdata = (CSR_ENDYNRATEREDUCTION_MASK | CSR_ROLLINTOCOARSE_MASK |
			   (0x3U << CSR_INCDECRATE_LSB));
		regdata = config->uia.disableretraining ?
			  regdata : (regdata | CSR_TRAINENRXEN_MASK);
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | TDBYTE |
								CSR_TRAININGPARAM_ADDR))),
			      regdata);

		/*
		 * - Register: Tsm0
		 *   - Fields:
		 *     - DtsmEnb
		 */
		regdata = (0x1U << CSR_DTSMENB_LSB);
		mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | I0 | TDBYTE |
								CSR_TSM0_ADDR))),
			      regdata);

		/*
		 * - Register: Tsm2
		 *   - Fields:
		 *     - DtsmDisErrChk
		 */
		regdata = (0x1U << CSR_DTSMDISERRCHK_LSB);
		for (vec = 1U; vec <= I_MAX; vec++) {
			i_addr = vec * I1;
			mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (c_addr | i_addr | TDBYTE |
									CSR_TSM2_ADDR))),
				      regdata);
		}
	}
}
#endif /* STM32MP_LPDDR4_TYPE */

/*
 * - Register: calrate
 *   - Fields:
 *     - calOnce
 *     - calinterval
 *   - Dependencies
 *     - user_input_advanced.calinterval
 *     - user_input_advanced.calonce
 */
static void calrate_program(struct stm32mp_ddr_config *config)
{
	uint32_t calinterval;
	uint32_t calonce;
	uint16_t calrate;

	calinterval = config->uia.calinterval;
	calonce = config->uia.calonce;

	calrate = (uint16_t)((0x1U << CSR_CALRUN_LSB) | (calonce << CSR_CALONCE_LSB) |
			     (calinterval << CSR_CALINTERVAL_LSB));

	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_CALRATE_ADDR))), calrate);
}

/*
 * Loads registers after training
 *
 * This function programs the PHY Initialization Engine (PIE) instructions and
 * the associated registers.
 * Training hardware registers are also programmed to for mission mode
 * retraining. (LPDDR4)
 *
 * \return void
 */
void ddrphy_phyinit_i_loadpieimage(struct stm32mp_ddr_config *config, bool skip_training)
{
	/*
	 * Enable access to the internal CSRs by setting the MicroContMuxSel CSR to 0.
	 * This allows the memory controller unrestricted access to the configuration CSRs.
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICROCONTMUXSEL_ADDR))),
		      0x0U);

	ddrphy_phyinit_loadpieprodcode();

#if STM32MP_LPDDR4_TYPE
	/*
	 * No user specified EnableDfiCsPolarityFix, running with new PUB with DFI CS polarity fix
	 * so program the data polarity CSR.
	 */
	dfiwrrddatacsconfig_program();
#endif /* STM32MP_LPDDR4_TYPE */

	seq0bdly_program(config);

	seq0bdisableflag_program(config, skip_training);

#if STM32MP_LPDDR4_TYPE
	if (!skip_training) {
		ppttrainsetup_program(config);
	}

	acsmplayback_program();

	traininghwreg_program(config);
#endif /* STM32MP_LPDDR4_TYPE */

	/*
	 * - Register: CalZap
	 *   - Prepare the calibration controller for mission mode.
	 *     Turn on calibration and hold idle until dfi_init_start is asserted sequence is
	 *     triggered.
	 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TMASTER | CSR_CALZAP_ADDR))), 0x1U);

	calrate_program(config);

	/*
	 * At the end of this function, PHY Clk gating register UcclkHclkEnables is
	 * set for mission mode.  Additionally APB access is Isolated by setting
	 * MicroContMuxSel.
	 */
	/* Disabling Ucclk (PMU) and Hclk (training hardware) */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TDRTUB | CSR_UCCLKHCLKENABLES_ADDR))),
		      0x0U);

	/* Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1 */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_MICROCONTMUXSEL_ADDR))),
		      0x1U);
}
