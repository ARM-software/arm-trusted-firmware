/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>

/*
 * This function implements the flow of PhyInit software to initialize the PHY.
 *
 * The execution sequence follows the overview figure provided in the Reference Manual.
 *
 * \returns 0 on completion of the sequence, EXIT_FAILURE on error.
 */
int ddrphy_phyinit_sequence(struct stm32mp_ddr_config *config, bool skip_training, bool reten)
{
	int ret;
	uint32_t ardptrinitval;	/*
				 * Represents the value stored in Step C into the register with the
				 * same name. Defined as a global variable so that implementation
				 * of ddrphy_phyinit_progcsrskiptrain() function does not require
				 * a PHY read register implementation.
				 */
	struct pmu_smb_ddr_1d mb_ddr_1d; /* Firmware 1D Message Block structure */

	/* Check user input pstate number consistency vs. SW capabilities */
	if (config->uib.numpstates > 1U) {
		return -1;
	}

	/* Initialize structures */
	ddrphy_phyinit_initstruct(config, &mb_ddr_1d);

	/* Re-calculate Firmware Message Block input based on final user input */
	ret = ddrphy_phyinit_calcmb(config, &mb_ddr_1d);
	if (ret != 0) {
		return ret;
	}

	/* (A) Bring up VDD, VDDQ, and VAA */
	/* ddrphy_phyinit_usercustom_a_bringuppower(); */

	/* (B) Start Clocks and Reset the PHY */
	/* ddrphy_phyinit_usercustom_b_startclockresetphy(); */

	/* (C) Initialize PHY Configuration */
	ret = ddrphy_phyinit_c_initphyconfig(config, &mb_ddr_1d, &ardptrinitval);
	if (ret != 0) {
		return ret;
	}
	/*
	 * Customize any register write desired; This can include any CSR not covered by PhyInit
	 * or user wish to override values calculated in step_C.
	 */
	ddrphy_phyinit_usercustom_custompretrain(config);

	/* Stop retention register tracking for training firmware related registers */
	ret = ddrphy_phyinit_reginterface(STOPTRACK, 0U, 0U);
	if (ret != 0) {
		return ret;
	}

	if (skip_training) {
		/* Skip running training firmware entirely */
		ddrphy_phyinit_progcsrskiptrain(config, &mb_ddr_1d, ardptrinitval);
	} else {
		/* (D) Load the IMEM Memory for 1D training */
		ddrphy_phyinit_d_loadimem();

		/* (E) Set the PHY input clocks to the desired frequency */
		/* ddrphy_phyinit_usercustom_e_setdficlk(pstate); */

		/* (F) Write the Message Block parameters for the training firmware */
		ret = ddrphy_phyinit_f_loaddmem(config, &mb_ddr_1d);
		if (ret != 0) {
			return ret;
		}

		/* (G) Execute the Training Firmware */
		ret = ddrphy_phyinit_g_execfw();
		if (ret != 0) {
			return ret;
		}

		/* (H) Read the Message Block results */
		/* ddrphy_phyinit_h_readmsgblock(); */
	}

	/* Start retention register tracking for training firmware related registers */
	ret = ddrphy_phyinit_reginterface(STARTTRACK, 0U, 0U);
	if (ret != 0) {
		return ret;
	}

	/* (I) Load PHY Init Engine Image */
	ddrphy_phyinit_i_loadpieimage(config, skip_training);

	/*
	 * Customize any CSR write desired to override values programmed by firmware or
	 * ddrphy_phyinit_i_loadpieimage()
	 */
	/* ddrphy_phyinit_usercustom_customposttrain(); */

	if (reten) {
		/* Save value of tracked registers for retention restore sequence. */
		ret = ddrphy_phyinit_usercustom_saveretregs(config);
		if (ret != 0) {
			return ret;
		}
	}

	/* (J) Initialize the PHY to Mission Mode through DFI Initialization */
	/* ddrphy_phyinit_usercustom_j_entermissionmode(); */

	return 0;
}
