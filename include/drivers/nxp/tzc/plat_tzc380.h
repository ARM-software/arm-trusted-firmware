/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if !defined(PLAT_TZC380_H) && defined(IMAGE_BL2)
#define PLAT_TZC380_H

#include <tzc380.h>

/* Number of DRAM regions to be configured
 * for the platform can be over-written.
 *
 * Array tzc400_reg_list too, needs be over-written
 * if there is any changes to default DRAM region
 * configuration.
 */
#ifndef MAX_NUM_TZC_REGION
/* 3 regions:
 *  Region 0(default),
 *  Region 1 (DRAM0, Secure Memory),
 *  Region 2 (DRAM0, Shared memory)
 */
#define MAX_NUM_TZC_REGION	3
#define DEFAULT_TZASC_CONFIG	1
#endif

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	uint64_t addr;
	uint64_t size;
	unsigned int sub_mask;
};

void mem_access_setup(uintptr_t base, uint32_t total_regions,
			struct tzc380_reg *tzc380_reg_list);

int populate_tzc380_reg_list(struct tzc380_reg *tzc380_reg_list,
			     int dram_idx, int list_idx,
			     uint64_t dram_start_addr,
			     uint64_t dram_size,
			     uint32_t secure_dram_sz,
			     uint32_t shrd_dram_sz);

#endif /* PLAT_TZC380_H */
