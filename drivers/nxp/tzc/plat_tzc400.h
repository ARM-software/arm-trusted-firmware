/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#if !defined(PLAT_TZC400_H) && defined(IMAGE_BL2)
#define PLAT_TZC400_H

#include <tzc400.h>

/* Structure to configure TZC Regions' boundaries and attributes. */
struct tzc400_reg {
	uint8_t reg_filter_en;
	unsigned long long start_addr;
	unsigned long long end_addr;
	unsigned int sec_attr;
	unsigned int nsaid_permissions;
};

#define TZC_REGION_NS_NONE	0x00000000U

/* NXP Platforms do not support NS Access ID (NSAID) based non-secure access.
 * Supports only non secure through generic NS ACCESS ID
 */
#define TZC_NS_ACCESS_ID	0xFFFFFFFFU

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
#define MAX_NUM_TZC_REGION	NUM_DRAM_REGIONS + 3
#define DEFAULT_TZASC_CONFIG	1
#endif

void mem_access_setup(uintptr_t base, uint32_t total_regions,
		      struct tzc400_reg *tzc400_reg_list);
int populate_tzc400_reg_list(struct tzc400_reg *tzc400_reg_list,
			     int dram_idx, int list_idx,
			     uint64_t dram_start_addr,
			     uint64_t dram_size,
			     uint32_t secure_dram_sz,
			     uint32_t shrd_dram_sz);

#endif /* PLAT_TZC400_H */
