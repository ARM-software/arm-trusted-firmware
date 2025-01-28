/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRAMC_H
#define DRAMC_H

#define DRAM_MAX_FREQ	16
#define DRAM_MAX_MR_CNT	10
#define DRAM_MAX_RK	2

int init_dramc_info(void *tag_entry);
int get_dram_step_freq(unsigned int step);
unsigned int get_dram_type(void);

enum dram_type {
	TYPE_DDR1 = 1,
	TYPE_LPDDR2,
	TYPE_LPDDR3,
	TYPE_PCDDR3,
	TYPE_LPDDR4,
	TYPE_LPDDR4X,
	TYPE_LPDDR4P,
	TYPE_LPDDR5,
	TYPE_LPDDR5X,
};
#endif /* DRAMC_H */
