/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include "xlat_mpu_private.h"

#include <fvp_r_arch_helpers.h>
#include <platform_def.h>

#warning "xlat_mpu library is currently experimental and its API may change in future."


void xlat_mmap_print(__unused const mmap_region_t *mmap)
{
	/* Empty */
}

#if LOG_LEVEL < LOG_LEVEL_VERBOSE

void xlat_tables_print(__unused xlat_ctx_t *ctx)
{
	/* Empty */
}

#else /* if LOG_LEVEL >= LOG_LEVEL_VERBOSE */

static void xlat_tables_print_internal(__unused xlat_ctx_t *ctx)
{
	int region_to_use = 0;
	uintptr_t region_base;
	size_t region_size;
	uint64_t prenr_el2_value = 0U;

	/*
	 * Keep track of how many invalid descriptors are counted in a row.
	 * Whenever multiple invalid descriptors are found, only the first one
	 * is printed, and a line is added to inform about how many descriptors
	 * have been omitted.
	 */

	/*
	 * TODO:  Remove this WARN() and comment when these API calls are more
	 *        completely implemented and tested!
	 */
	WARN("%s in this early version of xlat_mpu library may not produce reliable results!",
	     __func__);

	/*
	 * Sequence through all regions and print those in-use (PRENR has an
	 * enable bit for each MPU region, 1 for in-use or 0 for unused):
	 */
	prenr_el2_value = read_prenr_el2();
	for (region_to_use = 0;  region_to_use < N_MPU_REGIONS;
	     region_to_use++) {
		if (((prenr_el2_value >> region_to_use) & 1U) == 0U) {
			continue;
		}
		region_base = read_prbar_el2() & PRBAR_PRLAR_ADDR_MASK;
		region_size = read_prlar_el2() & PRBAR_PRLAR_ADDR_MASK;
		printf("Address:  0x%llx, size:  0x%llx ",
			(long long) region_base,
			(long long) region_size);
	}
}

void xlat_tables_print(__unused xlat_ctx_t *ctx)
{
	xlat_tables_print_internal(ctx);
}

#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */
