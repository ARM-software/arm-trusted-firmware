/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AMU_H__
#define __AMU_H__

#include <sys/cdefs.h> /* for CASSERT() */
#include <cassert.h>
#include <platform_def.h>

/* All group 0 counters */
#define AMU_GROUP0_COUNTERS_MASK	0xf

#ifdef PLAT_AMU_GROUP1_COUNTERS_MASK
#define AMU_GROUP1_COUNTERS_MASK	PLAT_AMU_GROUP1_COUNTERS_MASK
#else
#define AMU_GROUP1_COUNTERS_MASK	0
#endif

#ifdef PLAT_AMU_GROUP1_NR_COUNTERS
#define AMU_GROUP1_NR_COUNTERS		PLAT_AMU_GROUP1_NR_COUNTERS
#else
#define AMU_GROUP1_NR_COUNTERS		0
#endif

CASSERT(AMU_GROUP1_COUNTERS_MASK <= 0xffff, invalid_amu_group1_counters_mask);
CASSERT(AMU_GROUP1_NR_COUNTERS <= 16, invalid_amu_group1_nr_counters);

void amu_enable(int el2_unused);

#endif /* __AMU_H__ */
