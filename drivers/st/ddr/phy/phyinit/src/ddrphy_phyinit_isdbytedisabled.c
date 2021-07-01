/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>
#include <ddrphy_wrapper.h>

/*
 * Helper function to determine if a given DByte is Disabled given PhyInit inputs.
 * @return 1 if disabled, 0 if enabled.
 */
int ddrphy_phyinit_isdbytedisabled(struct stm32mp_ddr_config *config,
				   struct pmu_smb_ddr_1d *mb_ddr_1d, uint32_t dbytenumber)
{
	int disabledbyte;
	uint32_t nad0 __maybe_unused;
	uint32_t nad1 __maybe_unused;

	disabledbyte = 0; /* Default assume Dbyte is Enabled */

#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	disabledbyte = (dbytenumber > (config->uib.numactivedbytedfi0 - 1U)) ? 1 : 0;
#else /* STM32MP_LPDDR4_TYPE */
	nad0 = config->uib.numactivedbytedfi0;
	nad1 = config->uib.numactivedbytedfi1;

	if ((nad0 + nad1) > config->uib.numdbyte) {
		ERROR("%s %d\n", __func__, __LINE__);
		VERBOSE("%s invalid PHY configuration:\n", __func__);
		VERBOSE("numactivedbytedfi0(%u)+numactivedbytedfi1(%u)>numdbytes(%u).\n",
			nad0, nad1, config->uib.numdbyte);
	}

	if (config->uib.dfi1exists != 0U) {
		if (config->uib.numactivedbytedfi1 == 0U) {
			/* Only dfi0 (ChA) is enabled, dfi1 (ChB) disabled */
			disabledbyte = (dbytenumber > (config->uib.numactivedbytedfi0 - 1U)) ?
				       1 : 0;
		} else {
			/* DFI1 enabled */
			disabledbyte = (((config->uib.numactivedbytedfi0 - 1U) < dbytenumber) &&
					(dbytenumber < (config->uib.numdbyte / 2U))) ?
				       1 : (dbytenumber >
					    ((config->uib.numdbyte / 2U) +
					     config->uib.numactivedbytedfi1 - 1U)) ? 1 : 0;
		}
	} else {
		disabledbyte = (dbytenumber > (config->uib.numactivedbytedfi0 - 1U)) ? 1 : 0;
	}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	/* Qualify results against MessageBlock */
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	if ((mb_ddr_1d->enableddqs < 1U) ||
	    (mb_ddr_1d->enableddqs > (uint8_t)(8U * config->uib.numactivedbytedfi0))) {
		ERROR("%s %d\n", __func__, __LINE__);
		VERBOSE("%s enableddqs(%u)\n", __func__, mb_ddr_1d->enableddqs);
		VERBOSE("Value must be 0 < enableddqs < config->uib.numactivedbytedfi0 * 8.\n");
	}

	if (dbytenumber < 8) {
		disabledbyte |= (int)mb_ddr_1d->disableddbyte & (0x1 << dbytenumber);
	}
#else /* STM32MP_LPDDR4_TYPE */
	if ((mb_ddr_1d->enableddqscha < 1U) ||
	    (mb_ddr_1d->enableddqscha > (uint8_t)(8U * config->uib.numactivedbytedfi0))) {
		ERROR("%s %d\n", __func__, __LINE__);
		VERBOSE("%s enableddqscha(%u)\n", __func__, mb_ddr_1d->enableddqscha);
		VERBOSE("Value must be 0 < enableddqscha < config->uib.numactivedbytedfi0*8\n");
	}

	if ((config->uib.dfi1exists != 0U) && (config->uib.numactivedbytedfi1 > 0U) &&
	    ((mb_ddr_1d->enableddqschb < 1U) ||
	     (mb_ddr_1d->enableddqschb > (uint8_t)(8U * config->uib.numactivedbytedfi1)))) {
		ERROR("%s %d\n", __func__, __LINE__);
		VERBOSE("%s enableddqschb(%u)\n", __func__, mb_ddr_1d->enableddqschb);
		VERBOSE("Value must be 0 < enableddqschb < config->uib.numactivedbytedfi1*8\n");
	}
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */

	return disabledbyte;
}
