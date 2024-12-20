/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_STATS_H
#define MT_SPM_STATS_H

#include <stdint.h>

enum spm_stat_type {
	SPM_SLP_COUNT,
	SPM_SLP_DURATION,
};

enum spm_stat_state {
	SPM_STAT_MCUSYS,
	SPM_STAT_F26M,
	SPM_STAT_VCORE,
	SPM_STAT_D1_2,
	SPM_STAT_D2,
	SPM_STAT_D3,
	SPM_STAT_D4,
	SPM_STAT_D6X,
	NUM_SPM_STAT,
};

struct spm_lp_stat_record {
	uint64_t count;
	uint64_t duration;
};

struct spm_lp_stat {
	struct spm_lp_stat_record record[NUM_SPM_STAT];
};

void mt_spm_update_lp_stat(struct spm_lp_stat *stat);

uint64_t mt_spm_get_lp_stat(struct spm_lp_stat *stat,
			    uint32_t index, uint32_t type);

#endif /* MT_SPM_STATS_H */
