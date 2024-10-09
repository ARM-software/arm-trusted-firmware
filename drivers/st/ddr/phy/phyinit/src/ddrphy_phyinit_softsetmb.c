/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>

#include <ddrphy_phyinit.h>

/*
 * Set messageBlock variable only if not set by user
 *
 * This function is used by ddrphy_phyinit_calcmb() to set calculated
 * messageBlock variables only when the user has not directly programmed them.
 *
 * @param[in]   field   A string representing the messageBlock field to be programed.
 * @param[in]   value   filed value
 *
 * @return 0 on success.
 * On error  returns the following values based on error:
 * - -1 : message block field specified by the input \c field string is not
 * found in the message block data structure.
 */
int ddrphy_phyinit_softsetmb(struct pmu_smb_ddr_1d *mb_ddr_1d, enum message_block_field field,
			     uint32_t value)
{
	int ret = 0;

	if (field == MB_FIELD_DRAMFREQ) {
		assert(value <= UINT16_MAX);
	} else {
		assert(value <= UINT8_MAX);
	}

	switch (field) {
	case MB_FIELD_PSTATE:
		mb_ddr_1d->pstate = (uint8_t)value;
		break;
	case MB_FIELD_PLLBYPASSEN:
		mb_ddr_1d->pllbypassen = (uint8_t)value;
		break;
	case MB_FIELD_DRAMFREQ:
		mb_ddr_1d->dramfreq = (uint16_t)value;
		break;
	case MB_FIELD_DFIFREQRATIO:
		mb_ddr_1d->dfifreqratio = (uint8_t)value;
		break;
	case MB_FIELD_BPZNRESVAL:
		mb_ddr_1d->bpznresval = (uint8_t)value;
		break;
	case MB_FIELD_PHYODTIMPEDANCE:
		mb_ddr_1d->phyodtimpedance = (uint8_t)value;
		break;
	case MB_FIELD_PHYDRVIMPEDANCE:
		mb_ddr_1d->phydrvimpedance = (uint8_t)value;
		break;
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	case MB_FIELD_DRAMTYPE:
		mb_ddr_1d->dramtype = (uint8_t)value;
		break;
	case MB_FIELD_DISABLEDDBYTE:
		mb_ddr_1d->disableddbyte = (uint8_t)value;
		break;
	case MB_FIELD_ENABLEDDQS:
		mb_ddr_1d->enableddqs = (uint8_t)value;
		break;
	case MB_FIELD_PHYCFG:
		mb_ddr_1d->phycfg = (uint8_t)value;
		break;
#if STM32MP_DDR4_TYPE
	case MB_FIELD_X16PRESENT:
		mb_ddr_1d->x16present = (uint8_t)value;
		break;
#endif /* STM32MP_DDR4_TYPE */
#else /* STM32MP_LPDDR4_TYPE */
	case MB_FIELD_ENABLEDDQSCHA:
		mb_ddr_1d->enableddqscha = (uint8_t)value;
		break;
	case MB_FIELD_CSPRESENTCHA:
		mb_ddr_1d->cspresentcha = (uint8_t)value;
		break;
	case MB_FIELD_ENABLEDDQSCHB:
		mb_ddr_1d->enableddqschb = (uint8_t)value;
		break;
	case MB_FIELD_CSPRESENTCHB:
		mb_ddr_1d->cspresentchb = (uint8_t)value;
		break;
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
	default:
		ERROR("unknown message block field %u\n", field);
		ret = -1;
		break;
	}

	return ret;
}
