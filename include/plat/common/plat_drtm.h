/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DRTM_H
#define PLAT_DRTM_H

#include <stdint.h>
#include <lib/xlat_tables/xlat_tables_compat.h>

typedef struct {
	uint8_t max_num_mem_prot_regions;
	uint8_t dma_protection_support;
} plat_drtm_dma_prot_features_t;

typedef struct {
	bool tpm_based_hash_support;
	uint32_t firmware_hash_algorithm;
} plat_drtm_tpm_features_t;

typedef struct {
	uint64_t region_address;
	uint64_t region_size_type;
} __attribute__((packed)) drtm_mem_region_t;

/*
 * Memory region descriptor table structure as per DRTM beta0 section 3.13
 * Table 11 MEMORY_REGION_DESCRIPTOR_TABLE
 */
typedef struct {
	uint16_t revision;
	uint16_t reserved;
	uint32_t num_regions;
	drtm_mem_region_t region[];
} __attribute__((packed)) drtm_memory_region_descriptor_table_t;

/* platform specific address map functions */
const mmap_region_t *plat_get_addr_mmap(void);

/* platform-specific DMA protection functions */
bool plat_has_non_host_platforms(void);
bool plat_has_unmanaged_dma_peripherals(void);
unsigned int plat_get_total_smmus(void);
void plat_enumerate_smmus(const uintptr_t **smmus_out,
			  size_t *smmu_count_out);
const plat_drtm_dma_prot_features_t *plat_drtm_get_dma_prot_features(void);
uint64_t plat_drtm_dma_prot_get_max_table_bytes(void);

/* platform-specific TPM functions */
const plat_drtm_tpm_features_t *plat_drtm_get_tpm_features(void);

/*
 * TODO: Implement these functions as per the platform use case,
 * as of now none of the platform uses these functions
 */
uint64_t plat_drtm_get_min_size_normal_world_dce(void);
uint64_t plat_drtm_get_tcb_hash_table_size(void);
uint64_t plat_drtm_get_imp_def_dlme_region_size(void);
uint64_t plat_drtm_get_tcb_hash_features(void);

/* DRTM error handling functions */
int plat_set_drtm_error(uint64_t error_code);
int plat_get_drtm_error(uint64_t *error_code);

/*
 * Platform-specific function to ensure passed region lies within
 * Non-Secure region of DRAM
 */
int plat_drtm_validate_ns_region(uintptr_t region_start,
				 size_t region_size);

#endif /* PLAT_DRTM_H */
