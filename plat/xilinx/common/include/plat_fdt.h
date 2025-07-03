/*
 * Copyright (c) 2023-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef PLAT_FDT_H
#define PLAT_FDT_H

void prepare_dtb(void);
uintptr_t plat_retrieve_dt_addr(void);
int32_t is_valid_dtb(void *fdt);

#define MAX_RESERVE_ADDR_INDICES 32
struct reserve_mem_range {
	uintptr_t base;
	size_t size;
};

#if (TRANSFER_LIST == 1)
uint32_t retrieve_reserved_entries(void);
struct reserve_mem_range *get_reserved_entries_fdt(uint32_t *reserve_nodes);
#else
static inline uint32_t retrieve_reserved_entries(void)
{
	return 0;
}

static inline struct reserve_mem_range *get_reserved_entries_fdt(uint32_t *reserve_nodes)
{
	if (reserve_nodes) {
		*reserve_nodes = 0;
	}

	return NULL;
}
#endif

#endif /* PLAT_FDT_H */
