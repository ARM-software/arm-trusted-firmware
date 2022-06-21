/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_MAIN_H
#define DRTM_MAIN_H

#include <stdint.h>

#include <assert.h>
#include <lib/smccc.h>

#include "drtm_dma_prot.h"

#define ALIGNED_UP(x, a) __extension__ ({ \
	__typeof__(a) _a = (a); \
	__typeof__(a) _one = 1; \
	assert(IS_POWER_OF_TWO(_a)); \
	((x) + (_a - _one)) & ~(_a - _one); \
})

#define ALIGNED_DOWN(x, a) __extension__ ({ \
	__typeof__(a) _a = (a); \
	__typeof__(a) _one = 1; \
	assert(IS_POWER_OF_TWO(_a)); \
	(x) & ~(_a - _one); \
})

#define DRTM_PAGE_SIZE		(4 * (1 << 10))
#define DRTM_PAGE_SIZE_STR	"4-KiB"

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

struct __packed drtm_dl_args_v1 {
	uint16_t version;        /* Must be 1. */
	uint8_t __res[2];
	uint32_t features;
	uint64_t dlme_paddr;
	uint64_t dlme_size;
	uint64_t dlme_img_off;
	uint64_t dlme_img_ep_off;
	uint64_t dlme_img_size;
	uint64_t dlme_data_off;
	uint64_t dce_nwd_paddr;
	uint64_t dce_nwd_size;
	drtm_dl_dma_prot_args_v1_t dma_prot_args;
} __aligned(__alignof(uint16_t /* First member's type, `uint16_t version' */));

drtm_memory_region_descriptor_table_t *drtm_build_address_map(void);
uint64_t drtm_get_address_map_size(void);

/*
 * Version-independent type.  May be used to avoid excessive line of code
 * changes when migrating to new struct versions.
 */
typedef struct drtm_dl_args_v1 struct_drtm_dl_args;

#endif /* DRTM_MAIN_H */
