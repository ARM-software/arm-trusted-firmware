/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_DMA_PROT_H
#define DRTM_DMA_PROT_H

#include <stdint.h>
#include <plat/common/platform.h>
#include <services/drtm_svc.h>

struct __packed drtm_dl_dma_prot_args_v1 {
	uint64_t dma_prot_table_paddr;
	uint64_t dma_prot_table_size;
};

/* Values for DRTM_PROTECT_MEMORY */
enum dma_prot_type {
	PROTECT_NONE    = -1,
	PROTECT_MEM_ALL = 0,
	PROTECT_MEM_REGION = 2,
};

struct dma_prot {
	enum dma_prot_type type;
};

#define DRTM_MEM_REGION_PAGES_AND_TYPE(pages, type) \
	(((uint64_t)(pages) & (((uint64_t)1 << 52) - 1)) \
	 | (((uint64_t)(type) & 0x7) << 52))

#define PAGES_AND_TYPE(pages, type) \
		.region_size_type = DRTM_MEM_REGION_PAGES_AND_TYPE(pages, type)

/* Opaque / encapsulated type. */
typedef struct drtm_dl_dma_prot_args_v1 drtm_dl_dma_prot_args_v1_t;

bool drtm_dma_prot_init(void);
enum drtm_retc drtm_dma_prot_check_args(const drtm_dl_dma_prot_args_v1_t *a,
					int a_dma_prot_type,
					drtm_mem_region_t p);
enum drtm_retc drtm_dma_prot_engage(const drtm_dl_dma_prot_args_v1_t *a,
				    int a_dma_prot_type);
enum drtm_retc drtm_dma_prot_disengage(void);
uint64_t drtm_unprotect_mem(void *ctx);
void drtm_dma_prot_serialise_table(uint8_t *dst, size_t *size_out);

#endif /* DRTM_DMA_PROT_H */
