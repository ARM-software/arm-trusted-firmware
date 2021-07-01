/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDRPHY_PHYINIT_H
#define DDRPHY_PHYINIT_H

#include <stdbool.h>

#include <ddrphy_phyinit_usercustom.h>

enum message_block_field {
	MB_FIELD_PSTATE,
	MB_FIELD_PLLBYPASSEN,
	MB_FIELD_DRAMFREQ,
	MB_FIELD_DFIFREQRATIO,
	MB_FIELD_BPZNRESVAL,
	MB_FIELD_PHYODTIMPEDANCE,
	MB_FIELD_PHYDRVIMPEDANCE,
	MB_FIELD_DRAMTYPE,
	MB_FIELD_DISABLEDDBYTE,
	MB_FIELD_ENABLEDDQS,
	MB_FIELD_PHYCFG,
	MB_FIELD_X16PRESENT,
	MB_FIELD_ENABLEDDQSCHA,
	MB_FIELD_CSPRESENTCHA,
	MB_FIELD_ENABLEDDQSCHB,
	MB_FIELD_CSPRESENTCHB,
};

/* Function definitions */
int ddrphy_phyinit_softsetmb(struct pmu_smb_ddr_1d *mb_ddr_1d, enum message_block_field field,
			     uint32_t value);
void ddrphy_phyinit_initstruct(struct stm32mp_ddr_config *config, struct pmu_smb_ddr_1d *mb_ddr_1d);
#endif /* DDRPHY_PHYINIT_H */
