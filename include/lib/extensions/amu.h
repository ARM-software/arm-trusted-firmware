/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_H
#define AMU_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/cassert.h>
#include <lib/utils_def.h>

#include <platform_def.h>

/* All group 0 counters */
#define AMU_GROUP0_COUNTERS_MASK	U(0xf)
#define AMU_GROUP0_NR_COUNTERS		U(4)

#ifdef PLAT_AMU_GROUP1_COUNTERS_MASK
#define AMU_GROUP1_COUNTERS_MASK	PLAT_AMU_GROUP1_COUNTERS_MASK
#else
#define AMU_GROUP1_COUNTERS_MASK	U(0)
#endif

/* Calculate number of group 1 counters */
#if (AMU_GROUP1_COUNTERS_MASK	& (1 << 15))
#define	AMU_GROUP1_NR_COUNTERS		16U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 14))
#define	AMU_GROUP1_NR_COUNTERS		15U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 13))
#define	AMU_GROUP1_NR_COUNTERS		14U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 12))
#define	AMU_GROUP1_NR_COUNTERS		13U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 11))
#define	AMU_GROUP1_NR_COUNTERS		12U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 10))
#define	AMU_GROUP1_NR_COUNTERS		11U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 9))
#define	AMU_GROUP1_NR_COUNTERS		10U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 8))
#define	AMU_GROUP1_NR_COUNTERS		9U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 7))
#define	AMU_GROUP1_NR_COUNTERS		8U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 6))
#define	AMU_GROUP1_NR_COUNTERS		7U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 5))
#define	AMU_GROUP1_NR_COUNTERS		6U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 4))
#define	AMU_GROUP1_NR_COUNTERS		5U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 3))
#define	AMU_GROUP1_NR_COUNTERS		4U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 2))
#define	AMU_GROUP1_NR_COUNTERS		3U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 1))
#define	AMU_GROUP1_NR_COUNTERS		2U
#elif (AMU_GROUP1_COUNTERS_MASK	& (1 << 0))
#define	AMU_GROUP1_NR_COUNTERS		1U
#else
#define	AMU_GROUP1_NR_COUNTERS		0U
#endif

CASSERT(AMU_GROUP1_COUNTERS_MASK <= 0xffff, invalid_amu_group1_counters_mask);

struct amu_ctx {
	uint64_t group0_cnts[AMU_GROUP0_NR_COUNTERS];
#if __aarch64__
	/* Architected event counter 1 does not have an offset register. */
	uint64_t group0_voffsets[AMU_GROUP0_NR_COUNTERS-1];
#endif

#if AMU_GROUP1_NR_COUNTERS
	uint64_t group1_cnts[AMU_GROUP1_NR_COUNTERS];
#if __aarch64__
	uint64_t group1_voffsets[AMU_GROUP1_NR_COUNTERS];
#endif
#endif
};

unsigned int amu_get_version(void);
void amu_enable(bool el2_unused);

/* Group 0 configuration helpers */
uint64_t amu_group0_cnt_read(unsigned int idx);
void amu_group0_cnt_write(unsigned int idx, uint64_t val);

#if __aarch64__
uint64_t amu_group0_voffset_read(unsigned int idx);
void amu_group0_voffset_write(unsigned int idx, uint64_t val);
#endif

#if AMU_GROUP1_NR_COUNTERS
bool amu_group1_supported(void);

/* Group 1 configuration helpers */
uint64_t amu_group1_cnt_read(unsigned int idx);
void amu_group1_cnt_write(unsigned int idx, uint64_t val);
void amu_group1_set_evtype(unsigned int idx, unsigned int val);

#if __aarch64__
uint64_t amu_group1_voffset_read(unsigned int idx);
void amu_group1_voffset_write(unsigned int idx, uint64_t val);
#endif

#endif

#endif /* AMU_H */
