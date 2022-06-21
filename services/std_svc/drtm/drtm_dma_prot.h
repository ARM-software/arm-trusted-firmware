/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_DMA_PROT_H
#define DRTM_DMA_PROT_H

#include <stdint.h>

struct __packed drtm_dl_dma_prot_args_v1 {
	uint64_t dma_prot_table_paddr;
	uint64_t dma_prot_table_size;
};

/* Opaque / encapsulated type. */
typedef struct drtm_dl_dma_prot_args_v1 drtm_dl_dma_prot_args_v1_t;

bool drtm_dma_prot_init(void);

#endif /* DRTM_DMA_PROT_H */
