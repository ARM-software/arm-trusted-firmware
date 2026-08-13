/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stdio.h"
#include "stdlib.h"
#include <time.h>

#include <math.h>
#include "test_gpt_utils.h"

#include <arch_helpers.h>
#include "arm_def.h"
#include "lib/gpt_rme/gpt_rme.h"

#define BLOCK_DESC_GPI (ULL(0xF0))
#define BLOCK_DESC_GPI_SHIFT (4)
#define BLOCK_DESC_GPI_MASK 	(ULL(0xF0))
#define BLOCK_DESC_ENC	(0b0001)

#define TABLE_DESC_RES0_MASK 	(ULL(0xFFF0000000000FF0))
#define TABLE_DESC_ENC	(0b0011)

#define ENTRY_TYPE_MASK (ULL(0xF))
#define INVALID_L0_ENTRY (0xF)

#define NEXT_LEVEL_TABLE_ADDR_LSB (12)
#define INVALID_L1_ENTRY (0x2)

#define CONTIG_DESC_ENC (0b0001)
#define NUM_GPI_OPTIONS (sizeof(GPI_OPTIONS)/sizeof(unsigned int))

#define L0_GPTSZ_4K_MASK (1 << GPCCR_L0GPTSZ_SHIFT)

int s_values[] = {
	[GPCCR_L0GPTSZ_30BITS] = 30,
	[GPCCR_L0GPTSZ_34BITS] = 34,
	[GPCCR_L0GPTSZ_36BITS] = 36,
	[GPCCR_L0GPTSZ_39BITS] = 39
}; /* derived from L0GPTSZ */

int p_values[] = {
	[GPCCR_PGS_4K] = 12,
	[GPCCR_PGS_64K] = 16,
	[GPCCR_PGS_16K] = 14
}; /* derived from PGS, note that 0b10 = PGS_16K and 0b01 = PGS_64K */

unsigned int GPI_OPTIONS[] = {TEST_GPI_NO_ACCESS, TEST_GPI_SECURE, TEST_GPI_NS,
				TEST_GPI_ROOT, TEST_GPI_REALM, TEST_GPI_ANY};

void enable_mmu(unsigned int flags)
{
	/* Enable MMU and data caches */
	write_sctlr_el3(read_sctlr_el3() | SCTLR_C_BIT);
}

void disable_mmu(void)
{
	write_sctlr_el3(read_sctlr_el3() & ~SCTLR_C_BIT);
}

void set_pps(gpccr_pps_e pps)
{
	u_register_t mask;
	u_register_t value;
	mask = ~(GPCCR_PPS_MASK << GPCCR_PPS_SHIFT);
	value = read_gpccr_el3() & mask;
	value |= SET_GPCCR_PPS(pps);
	write_gpccr_el3(value);
}

gpccr_pps_e get_pps(void)
{
	return (gpccr_pps_e)((read_gpccr_el3() & GPCCR_PPS_MASK) >> GPCCR_PPS_SHIFT);
}

void set_l0gptsz(gpccr_l0gptsz_e l0gptsz)
{
	u_register_t mask;
	u_register_t value;
	mask = ~(GPCCR_L0GPTSZ_MASK << GPCCR_L0GPTSZ_SHIFT);
	value = read_gpccr_el3() & mask;
	value |= (u_register_t)l0gptsz << GPCCR_L0GPTSZ_SHIFT;
	write_gpccr_el3(value);
}

gpccr_l0gptsz_e get_l0gptsz(void)
{
	return (gpccr_l0gptsz_e)((read_gpccr_el3() & GPCCR_L0GPTSZ_MASK) >> GPCCR_L0GPTSZ_SHIFT);
}

void set_pgs(gpccr_pgs_e pgs)
{
	u_register_t mask;
	u_register_t value;
	mask = ~(GPCCR_PGS_MASK << GPCCR_PGS_SHIFT);
	value = read_gpccr_el3() & mask;
	value |= SET_GPCCR_PGS(pgs);
	write_gpccr_el3(value);
}

gpccr_pgs_e get_pgs(void)
{
	return (gpccr_pgs_e)((read_gpccr_el3() & GPCCR_PGS_MASK) >> GPCCR_PGS_SHIFT);
}

size_t generate_random_size(unsigned long max_size, unsigned long pgs)
{
	unsigned long num_granules;
	num_granules = (generate_random_ulong() % (max_size / pgs)) + 1;
	return num_granules * pgs;
}

unsigned long generate_random_ulong(void)
{
	int rand_bits;
	int ulong_bits;
	unsigned long value;
	int i;
	int temp;

	rand_bits = ceil(log(RAND_MAX) / log(2));
	ulong_bits = (int)(sizeof(unsigned long)*8);
	value = (rand() % RAND_MAX);

	i = rand_bits;
	while (i < ulong_bits) {
		value = value << rand_bits;
		temp = (rand() % RAND_MAX);
		value = value | temp;
		i += rand_bits;
	}

	if ((ulong_bits - i) != 0) {
		value = value << (ulong_bits - i);
		value = value | (rand() % RAND_MAX);
	}

	return value;
}

bool is_valid_table_desc(uint64_t table_desc)
{
	uint64_t td_alignment_mask;
	uint64_t s;
	uint64_t p;
	size_t k;

	/* Check that all bits marked RES0 are 0 */
	if (table_desc & TABLE_DESC_RES0_MASK) {
		return false;
	}

	/* Check alignment of next level table descriptor */
	s = s_values[get_l0gptsz()];
	p = p_values[get_pgs()];
	/* Compute the size of the next level table (2^k) */
	k = s - p - 1;

	/* Alignment mask for bits [x:12], where x <= 51 */
	td_alignment_mask = ((1 << k) - 1) - ((1 << NEXT_LEVEL_TABLE_ADDR_LSB) - 1);
	if (table_desc & td_alignment_mask) {
		return false;
	}

	/*
	 * TODO validate next level table address if want to call
	 * walk_l0_table after L1 initialization. Otherwise not needed.
	 * L1 table does not exist until gpt_init_pas_regions is called,
	 * so if walk_l0_table is called before that, it does not make
	 * sense to try and validate the NLTA other than its alignment
	 */

	/* Check for table_descriptor encoding */
	if ((table_desc & ENTRY_TYPE_MASK) ^ TABLE_DESC_ENC) {
		return false;
	}

	return true;
}

bool is_valid_block_desc(uint64_t block_desc)
{
	uint64_t gpi_encoding;

	/* Check for block_descriptor encoding */
	if ((block_desc & ENTRY_TYPE_MASK) ^ BLOCK_DESC_ENC) {
		return false;
	}

	/* Check that the GPI encoding is valid. Values can be:
	 * 0x0, 0x8 to 0xB, 0xF */
	gpi_encoding = (block_desc & BLOCK_DESC_GPI_MASK) >> BLOCK_DESC_GPI_SHIFT;
	if ((gpi_encoding && (gpi_encoding < 0x8)) ||
	    ((gpi_encoding > 0xB) && (gpi_encoding < 0xF))) {
		return false;
	}

	return true;
}

/* If bits [3:0] of a level 1 GPT entry are a valid GPI encoding, the entry is a GPT Granules descriptor. */
bool is_valid_granules_desc(uint64_t granules_desc)
{
	int gpi_encoding = (int)(granules_desc & ENTRY_TYPE_MASK);

	switch (gpi_encoding) {
	case TEST_GPI_NO_ACCESS:
		break;
	case TEST_GPI_SECURE:
		break;
	case TEST_GPI_NS:
		break;
	case TEST_GPI_ROOT:
		break;
	case TEST_GPI_REALM:
		break;
	case TEST_GPI_ANY:
		break;
	default:
		return false;
	}

	return true;
}

/* If bits [3:0] of a level 1 GPT entry are 0b0001, the entry is a GPT Contiguous descriptor */
bool is_valid_contiguous_desc(uint64_t contiguous_desc)
{
	if ((contiguous_desc & ENTRY_TYPE_MASK) ^ CONTIG_DESC_ENC) {
		return false;
	}
	/*TODO finish */
	return true;
}

bool check_gpi_type(uint64_t gpi, unsigned int expected_type)
{
	if (((gpi & BLOCK_DESC_GPI_MASK) >> BLOCK_DESC_GPI_SHIFT) ^ expected_type) {
		return false;
	}

	return true;
}

uintptr_t allocate_l0_table_mem(size_t l0_mem_size)
{
	size_t alignment;
	size_t size;
	int num_entries;
	uint64_t *entry;
	uintptr_t table;

	/* Malloc table space */
	alignment = (l0_mem_size > MIN_L0_ALIGNMENT) ? l0_mem_size : MIN_L0_ALIGNMENT;

	size = (l0_mem_size / alignment) * alignment;
	if (l0_mem_size % alignment != 0) {
		size += alignment;
	}

	table = (uintptr_t)aligned_alloc(alignment, size);

	if (table) {
		/* Initialize entries as invalid */
		num_entries = size / L0_ENTRY_SIZE;
		entry = (uint64_t *)table;

		for (int i = 0; i < num_entries; i++) {
			entry[i] = INVALID_L0_ENTRY;
		}
	}
	return table;
}

uintptr_t allocate_l1_table_mem(uintptr_t table, size_t l1_mem_size)
{
	size_t alignment;
	int num_entries;
	uint64_t *entry;

	/* Allocate table space */
	table = (uintptr_t)aligned_alloc(l1_mem_size, l1_mem_size);

	if (table) {
		/* Initialize entries as invalid */
		num_entries = l1_mem_size / L1_ENTRY_SIZE;
		entry = (uint64_t *)table;

		for (int i = 0; i < num_entries; i++) {
			entry[i] = INVALID_L1_ENTRY;
		}
	}

	return table;
}

bool walk_l1_table(uintptr_t table, size_t l1_mem_size, bool l1_enabled,
		   pas_region_t *pas_regions, unsigned int pas_count)
{
	int num_entries;
	uint64_t *entry;

	num_entries = l1_mem_size / L1_ENTRY_SIZE;
	entry = (uint64_t *)table;

	for (int i = 0; i < num_entries; i++) {
		/* Check if valid granules descriptor or contiguous descriptor */
		if (is_valid_granules_desc(entry[i]) == false) {
			return false;
		} else if (is_valid_contiguous_desc(entry[i]) == false) {
			return false;
		}
	}

	return true;
}

/*
 * Creates a set of pass regions and returns the number of
 * regions created and the number of l1 tables that need
 * to be allocated.
 */
pas_regions_info_t create_pas_regions(pas_region_t *pas_regions,
	unsigned long pps, unsigned long pgs, unsigned long l0gptsz)
{
	size_t size;
	unsigned int attrs;
	int num_l0_entries;
	int entry_type;
	bool choose_entry_type = true;
	int i = 0;
	unsigned long int remaining_space = pps;
	srand(time(NULL));
	pas_regions_info_t counts = {
		.pas_count = 0,
		.l1_table_count = 0
	};
	uintptr_t base_pa = 0x0;
	num_l0_entries = pps/l0gptsz;
	for (i = 0; i < num_l0_entries; ++i) {
		bool entry_added = false;
		pas_region_t new_pas_region;
		if (choose_entry_type) {
			entry_type = i % 2;
		}
		choose_entry_type = true;
		/* Choose a GPI encoding for the entry */
		attrs = GPI_OPTIONS[(int)(i % NUM_GPI_OPTIONS)];

		if (entry_type == 1) { /* BLOCK DESC, no L1 */
			/*
			 * If base not on L0 edge, fill in the necessary space
			 * with a table descriptor so the subsequent block
			 * descriptor is aligned.
			 */
			if (base_pa % l0gptsz != 0) {
				size = l0gptsz - base_pa % l0gptsz;
				new_pas_region =
					(pas_region_t)GPT_MAP_REGION_GRANULE((base_pa),
						size, attrs);
				counts.l1_table_count++;
				entry_added = true;
				/*
				 * Prevent the entry type in the next iteration from
				 * being selected randomly, as it has to be a block
				 * descriptor.
				 */
				choose_entry_type = false;
			}
			/*
			 * If there is space in the protected physical
			 * memory space, add block descriptor.
			 */
			else if (base_pa + l0gptsz <= pps) {
				/* Create block PAS region on next L0 boundary */
				size = l0gptsz;
				new_pas_region =
					(pas_region_t)GPT_MAP_REGION_BLOCK((base_pa),
						l0gptsz, attrs);
				entry_added = true;
			}
		}
		} else { /* TABLE DESC, yes L1 */
			unsigned long int available_space =
				remaining_space > l0gptsz ? l0gptsz : remaining_space;
			size = pgs * 2;
			new_pas_region =
				(pas_region_t)GPT_MAP_REGION_GRANULE((base_pa), size,
					attrs);
			counts.l1_table_count++;
			entry_added = true;
		}
		if (entry_added) {
			pas_regions[counts.pas_count] = new_pas_region;
			base_pa += size;
			counts.pas_count++;
			remaining_space = pps - base_pa;
		}
	}
	return counts;
}
