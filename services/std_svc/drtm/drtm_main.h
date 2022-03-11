/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_MAIN_H
#define DRTM_MAIN_H

#include <stdint.h>

#include <lib/smccc.h>

#include "drtm_dma_prot.h"

enum drtm_retc {
	SUCCESS = SMC_OK,
	NOT_SUPPORTED = SMC_UNK,
	INVALID_PARAMETERS = -2,
	DENIED = -3,
	NOT_FOUND = -4,
	INTERNAL_ERROR = -5,
	MEM_PROTECT_INVALID = -6,
};

typedef struct {
	uint64_t tpm_features;
	uint64_t minimum_memory_requirement;
	uint64_t dma_prot_features;
	uint64_t boot_pe_id;
	uint64_t tcb_hash_features;
} drtm_features_t;

drtm_memory_region_descriptor_table_t *drtm_build_address_map(void);
uint64_t drtm_get_address_map_size(void);

#endif /* DRTM_MAIN_H */
