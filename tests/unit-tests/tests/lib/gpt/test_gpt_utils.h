/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long size_t;

#include <stdbool.h>
#include <stdint.h>
#include <lib/gpt_rme/gpt_rme.h>

extern unsigned int plat_l0gptsz;

/* Using TEST prefix to distinguish from macros defined in GPT library. */
#define TEST_GPI_NO_ACCESS (0x0)
#define TEST_GPI_SECURE (0x8)
#define TEST_GPI_NS 	(0x9)
#define TEST_GPI_ROOT 	(0xA)
#define TEST_GPI_REALM	(0xB)
#define TEST_GPI_ANY 	(0xF)

#define L0_ENTRY_SIZE	(8)
#define L1_ENTRY_SIZE	(8)

#define MIN_L0_ALIGNMENT 4096

/* Used to allocate space for pas_regions array */
#define PAS_ARRAY_MAX_SIZE (0x80000000)
#define PAS_COUNT_MAX	(PAS_ARRAY_MAX_SIZE / sizeof(pas_region_t))

typedef struct pas_regions_info {
	unsigned int pas_count;
	unsigned int l1_table_count;
} pas_regions_info_t;

void enable_mmu(unsigned int flags);
void disable_mmu(void);
void set_pps(gpccr_pps_e pps);
void set_l0gptsz(gpccr_l0gptsz_e l0gptsz);
void set_pgs(gpccr_pgs_e pgs);
size_t generate_random_size(unsigned long max_size, unsigned long pgs);
unsigned long generate_random_ulong(void);
bool is_valid_table_desc(uint64_t table_desc);
bool is_valid_block_desc(uint64_t block_desc);
bool is_valid_granules_desc(uint64_t granules_desc);
bool is_valid_contiguous_desc(uint64_t contiguous_desc);
bool check_gpi_type(uint64_t gpi, unsigned int expected_type);
bool walk_l1_table(uintptr_t table, size_t l1_mem_size, bool l1_enabled,
		   pas_region_t *pas_regions, unsigned int pas_count);
uintptr_t allocate_l0_table_mem(size_t l0_mem_size);
uintptr_t allocate_l1_table_mem(uintptr_t table, size_t l1_mem_size);
pas_regions_info_t create_pas_regions(pas_region_t *pas_regions,
				      unsigned long pps,
				      unsigned long pgs,
				      unsigned long l0gptsz);

#ifdef __cplusplus
}
#endif
