/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_H
#define AMU_H

#include <stdbool.h>
#include <stdint.h>

#include <platform_def.h>

#include <lib/cassert.h>
#include <lib/utils_def.h>

/* All group 0 counters */
#define AMU_GROUP0_COUNTERS_MASK	U(0xf)

#ifdef PLAT_AMU_GROUP1_COUNTERS_MASK
#define AMU_GROUP1_COUNTERS_MASK	PLAT_AMU_GROUP1_COUNTERS_MASK
#else
#define AMU_GROUP1_COUNTERS_MASK	U(0)
#endif

#ifdef PLAT_AMU_GROUP1_NR_COUNTERS
#define AMU_GROUP1_NR_COUNTERS		PLAT_AMU_GROUP1_NR_COUNTERS
#else
#define AMU_GROUP1_NR_COUNTERS		U(0)
#endif

CASSERT(AMU_GROUP1_COUNTERS_MASK <= 0xffff, invalid_amu_group1_counters_mask);
CASSERT(AMU_GROUP1_NR_COUNTERS <= 16, invalid_amu_group1_nr_counters);

bool amu_supported(void);
void amu_enable(bool el2_unused);

/* Group 0 configuration helpers */
uint64_t amu_group0_cnt_read(int idx);
void amu_group0_cnt_write(int idx, uint64_t val);

/* Group 1 configuration helpers */
uint64_t amu_group1_cnt_read(int idx);
void amu_group1_cnt_write(int idx, uint64_t val);
void amu_group1_set_evtype(int idx, unsigned int val);

#endif /* AMU_H */
