/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <mt_spm.h>
#include <mt_spm_reg.h>
#include <mt_spm_stats.h>

#define READ_AND_MASK_16BIT(addr)	(mmio_read_32(addr) & 0xFFFF)

void mt_spm_update_lp_stat(struct spm_lp_stat *stat)
{
	if (!stat)
		return;

	stat->record[SPM_STAT_MCUSYS].count += 1;
	stat->record[SPM_STAT_MCUSYS].duration +=
			mmio_read_32(SPM_BK_PCM_TIMER);
	stat->record[SPM_STAT_D1_2].count +=
			READ_AND_MASK_16BIT(SPM_APSRC_EVENT_COUNT_STA);
	stat->record[SPM_STAT_D2].count +=
			READ_AND_MASK_16BIT(SPM_EMI_EVENT_COUNT_STA);
	stat->record[SPM_STAT_D3].count +=
			READ_AND_MASK_16BIT(SPM_VRF18_EVENT_COUNT_STA);
	stat->record[SPM_STAT_D4].count +=
			READ_AND_MASK_16BIT(SPM_INFRA_EVENT_COUNT_STA);
	stat->record[SPM_STAT_D6X].count +=
			READ_AND_MASK_16BIT(SPM_PMIC_EVENT_COUNT_STA);
	stat->record[SPM_STAT_F26M].count +=
			READ_AND_MASK_16BIT(SPM_SRCCLKENA_EVENT_COUNT_STA);
	stat->record[SPM_STAT_F26M].duration +=
			READ_AND_MASK_16BIT(SPM_BK_VTCXO_DUR);
	stat->record[SPM_STAT_VCORE].count +=
			READ_AND_MASK_16BIT(SPM_VCORE_EVENT_COUNT_STA);
	stat->record[SPM_STAT_VCORE].duration +=
			mmio_read_32(SPM_SW_RSV_4);
}

uint64_t mt_spm_get_lp_stat(struct spm_lp_stat *stat,
			    uint32_t index, uint32_t type)
{

	uint64_t ret;

	if (!stat || index >= NUM_SPM_STAT)
		return 0;

	switch (type) {
	case SPM_SLP_COUNT:
		ret = stat->record[index].count;
		break;
	case SPM_SLP_DURATION:
		ret = stat->record[index].duration;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}
