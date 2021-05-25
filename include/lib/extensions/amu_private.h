/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_PRIVATE_H
#define AMU_PRIVATE_H

#include <stdint.h>

#include <lib/cassert.h>
#include <lib/extensions/amu.h>
#include <lib/utils_def.h>

#include <platform_def.h>

#define AMU_GROUP0_MAX_COUNTERS		U(16)
#define AMU_GROUP1_MAX_COUNTERS		U(16)

#if ENABLE_AMU_AUXILIARY_COUNTERS
#define AMU_GROUP1_COUNTERS_MASK	U(0)
#endif

struct amu_ctx {
	uint64_t group0_cnts[AMU_GROUP0_MAX_COUNTERS];
#if __aarch64__
	/* Architected event counter 1 does not have an offset register. */
	uint64_t group0_voffsets[AMU_GROUP0_MAX_COUNTERS-1];
#endif

#if ENABLE_AMU_AUXILIARY_COUNTERS
	uint64_t group1_cnts[AMU_GROUP1_MAX_COUNTERS];
#if __aarch64__
	uint64_t group1_voffsets[AMU_GROUP1_MAX_COUNTERS];
#endif
#endif
};

uint64_t amu_group0_cnt_read_internal(unsigned int idx);
void amu_group0_cnt_write_internal(unsigned int idx, uint64_t val);

#if ENABLE_AMU_AUXILIARY_COUNTERS
uint64_t amu_group1_cnt_read_internal(unsigned int idx);
void amu_group1_cnt_write_internal(unsigned int idx, uint64_t val);
void amu_group1_set_evtype_internal(unsigned int idx, unsigned int val);
#endif

#if __aarch64__
uint64_t amu_group0_voffset_read_internal(unsigned int idx);
void amu_group0_voffset_write_internal(unsigned int idx, uint64_t val);

#if ENABLE_AMU_AUXILIARY_COUNTERS
uint64_t amu_group1_voffset_read_internal(unsigned int idx);
void amu_group1_voffset_write_internal(unsigned int idx, uint64_t val);
#endif
#endif

#endif /* AMU_PRIVATE_H */
