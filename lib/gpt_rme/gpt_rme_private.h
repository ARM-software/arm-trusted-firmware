/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_RME_PRIVATE_H
#define GPT_RME_PRIVATE_H

#include <lib/gpt_rme/gpt_rme.h>
#include <lib/utils_def.h>

/*
 * Internal structure to retrieve the values from get_gpi_params();
 */
typedef struct {
	uint64_t gpt_l1_desc;
	uint64_t *gpt_l1_addr;
	unsigned int idx;
	unsigned int gpi_shift;
	unsigned int gpi;
#if (RME_GPT_BITLOCK_BLOCK != 0)
	bitlock_t *lock;
	LOCK_TYPE mask;
#endif
} gpi_info_t;

/*
 * Look up structure for contiguous blocks and descriptors
 */
typedef struct {
	size_t size;
	unsigned int desc;
} gpt_fill_lookup_t;

typedef void (*gpt_shatter_func)(uintptr_t base, const gpi_info_t *gpi_info,
					uint64_t l1_desc);
typedef void (*gpt_tlbi_func)(uintptr_t base);

/*
 * Look-up structure for
 * invalidating TLBs of GPT entries by Physical address, last level.
 */
typedef struct {
	gpt_tlbi_func function;
	size_t mask;
} gpt_tlbi_lookup_t;

#endif /* GPT_RME_PRIVATE_H */
