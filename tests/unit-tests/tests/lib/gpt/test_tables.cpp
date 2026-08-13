/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"
extern "C" {
#include <lib/gpt_rme/gpt_rme.h>
#include "arm_def.h"
#include <stdlib.h>
#include "test_gpt_def.h"
#include "test_gpt_utils.h"
#include <time.h>
#include <arch_helpers.h>
#include <errno.h>
}

gpccr_pps_e pps_encodings[] = {
	GPCCR_PPS_4GB,
	GPCCR_PPS_64GB,
	GPCCR_PPS_1TB,
	GPCCR_PPS_4TB,
	GPCCR_PPS_16TB,
	GPCCR_PPS_256TB,
	GPCCR_PPS_4PB
};

unsigned long int pps_values[] = {
	SIZE_4GB,
	SIZE_64GB,
	SIZE_1TB,
	SIZE_4TB,
	SIZE_16TB,
	SIZE_256TB,
	SIZE_4PB
};

gpccr_l0gptsz_e l0gptsz_encodings[] = {
	GPCCR_L0GPTSZ_30BITS,
	GPCCR_L0GPTSZ_34BITS,
	GPCCR_L0GPTSZ_36BITS,
	GPCCR_L0GPTSZ_39BITS
};

unsigned long int l0gptsz_values[] = {
	SIZE_1GB,
	SIZE_16GB,
	SIZE_64GB,
	SIZE_512GB
};

gpccr_pgs_e pgs_encodings[] = {
	GPCCR_PGS_4K,
	GPCCR_PGS_64K,
	GPCCR_PGS_16K
};

unsigned long int pgs_values[] = {
	SIZE_4KB,
	SIZE_16KB,
	SIZE_64KB
};

/*
 * TEST GROUP: gpt_init_l0_tables
 *	Test whether L0 tables are properly handled.
 */
TEST_GROUP(gpt_l0_tables) {
	gpccr_pps_e pps_encoding;
	size_t l0_mem_size;
	uintptr_t l0_table = 0;
	gpccr_pgs_e pgs_encoding;
	unsigned long pps;
	unsigned long l0gptsz;
	unsigned int flags;

	TEST_SETUP() {
		int i, j, k;

		/* Enable MMU and data caches */
		enable_mmu(flags);

		/* Get PPS */
		j = 0;
		pps = pps_values[j];
		pps_encoding = pps_encodings[j];

		/* Set the mock PPS */
		set_pps(pps_encoding);

		/* Get L0GPTSZ */
		i = 0;
		l0gptsz = l0gptsz_values[i];

		/* Set the mock L0 page size */
		set_l0gptsz(l0gptsz_encodings[i]);

		/* Get PGS */
		k = 0;
		pgs_encoding = pgs_encodings[k];

		/* Set the mock PGS */
		set_pgs((gpccr_pgs_e)pgs_encoding);

		/* Compute size for L0 table */
		l0_mem_size = pps > l0gptsz
			? (pps / l0gptsz) * L0_ENTRY_SIZE : L0_ENTRY_SIZE;
	}

	TEST_TEARDOWN() {
		if (l0_table) {
			free((void *)l0_table);
		}
	}
};

static void walk_l0_table(uintptr_t table, size_t l0_mem_size, bool l1_enabled)
{
	int num_entries;
	uint64_t * entry;

	num_entries = l0_mem_size / L0_ENTRY_SIZE;
	entry = (uint64_t *)table;

	for (int i = 0; i < num_entries; i++) {
		/* Check if valid L0 block descriptor */
		/* if L1 enabled, check for either block descriptor or table descriptor */
		if(is_valid_block_desc(entry[i])) {
			CHECK_EQUAL(true, check_gpi_type(entry[i], TEST_GPI_ANY));
			/* TODO other types/variable accding to pas_regions */
		}
		else if (l1_enabled && is_valid_table_desc(entry[i])) {
			continue;
		}
		else{
			FAIL("No valid descriptor found");
		}
	}
}

/*
 * TEST: init_and_walk
 *	Initializes the L0 table with all invalid entries and using
 *	the setup parameters, then walks through all its entries.
 */
TEST(gpt_l0_tables, init_and_walk) {
	/* Allocate memory for L0 */
	CHECK_TEXT(l0_table = allocate_l0_table_mem(l0_mem_size),
		"Unable to allocate L0 table.");
	CHECK(gpt_init_l0_tables(pps_encoding, l0_table, l0_mem_size) == 0);
	walk_l0_table(l0_table, l0_mem_size, false);
}

/*
 * TEST: unaligned
 *	Allocated an unaligned table and attempts to initialize it as an
 *	L0 table.
 */
TEST(gpt_l0_tables, unaligned){
	/* Determine size of table */
	l0_mem_size = (pps / l0gptsz) * L0_ENTRY_SIZE; /* 4GB/1GB * 8 = 32B */

	/* Allocate table space, unaligned */
	bool aligned = true;
	do {
		l0_table = (uintptr_t)calloc(1,l0_mem_size);
		/* if the allocated table is unaligned, stop */
		if ((l0_table & (SIZE_4KB - 1)) && (l0_table & (l0_mem_size - 1))) {
			aligned = false;
		}
		else if (l0_table) {
			free((void*)l0_table);
		}
	} while (l0_table && aligned);

	if (!l0_table) {
		FAIL("Could not allocate L0 table\n");
	}

	CHECK(gpt_init_l0_tables(pps_encoding, l0_table, l0_mem_size)
		== -EFAULT);
}

/*
 * TEST: size_lt_4k
 *	Given a table size < 4K, the table should be aligned to 4K. This
 *	test sets alignment to the table size and sees if the init
 *	function fails as it should.
 */
TEST(gpt_l0_tables, size_lt_4K){
	size_t alignment;

	/* Determine size of table */
	l0_mem_size = (SIZE_4GB/l0gptsz) * L0_ENTRY_SIZE;
	/* 4GB/1GB * 8 = 32B */

	alignment = (l0_mem_size < SIZE_4KB) ? l0_mem_size : SIZE_4KB;

	/* Malloc table space, misaligned to table size */
	l0_table = (uintptr_t)aligned_alloc(alignment,l0_mem_size);

	if(!l0_table) {
		FAIL("Could not allocate L0 table\n");
	}

	CHECK(gpt_init_l0_tables(pps_encoding, l0_table,
		l0_mem_size) == -EFAULT);
}

/*
 * TEST: size_gt_4k
 *	Given a table size > 4K, the table should be aligned to the
 *	table size. This test sets alignment to 4K and sees if the
 *	init function fails as it should.
 */
TEST(gpt_l0_tables, size_gt_4K){
	size_t alignment;
	uintptr_t actual_table;

	/* Get PPS */
	pps = SIZE_1TB;
	pps_encoding = GPCCR_PPS_1TB;

	/* Set the mock PPS */
	set_pps(pps_encoding);

	/* Determine size of table */
	l0_mem_size = (pps/l0gptsz) * L0_ENTRY_SIZE; /* 1TB/1GB * 8 = 8KB */
	alignment = (l0_mem_size < SIZE_4KB) ? l0_mem_size : SIZE_4KB;

	/* Allocate table space, misaligned to 4K */
	l0_table = (uintptr_t)aligned_alloc(alignment, l0_mem_size + SIZE_4KB);

	if(!l0_table) {
		FAIL("Could not allocate L0 table\n");
	}

	actual_table = l0_table;
	if (l0_table % l0_mem_size == 0){
		actual_table = l0_table + (SIZE_4KB/sizeof(uintptr_t));
	}
	CHECK(gpt_init_l0_tables(pps_encoding, actual_table,
		l0_mem_size) == -EFAULT);
}

/*
 * TEST: not_enough_memory
 *	Tests whether a table allocated with the wrong size can be
 *	properly initialized.
 */
TEST(gpt_l0_tables, not_enough_memory) {
	size_t correct_l0_mem_size;
	size_t alignment;
	size_t wrong_l0_mem_size;

	/*
	 * Calculate correct l0_mem_size for alignment purposes to ensure
	 * test fails due to not enough memory, not due to an invalid
	 * base address.
	 */
	correct_l0_mem_size = pps >= l0gptsz ? (pps / l0gptsz)
		* L0_ENTRY_SIZE : L0_ENTRY_SIZE;

	alignment = (correct_l0_mem_size > MIN_L0_ALIGNMENT)
		? correct_l0_mem_size : MIN_L0_ALIGNMENT;

	wrong_l0_mem_size = 1;

	l0_table = (uintptr_t)aligned_alloc(alignment, wrong_l0_mem_size);

	if (!l0_table) {
		FAIL("Unable to allocate L0 table\n");
	}

	CHECK(gpt_init_l0_tables(pps_encoding, l0_table,
		wrong_l0_mem_size) == -ENOMEM);
}

TEST_GROUP(gpt_init_pas_l1_tables) {
	TEST_SETUP() {
		int i, j, k;

		unsigned int num_l0_entries;
		unsigned int num_l1_tables;
		unsigned int init_pas_count;
		unsigned int flags;

		/* Enable MMU and data caches */
		enable_mmu(flags);

		uintptr_t base_pa = 0x0;

		/* Get PPS */
		j = 0;
		pps = pps_values[j];
		pps_encoding = pps_encodings[j];

		/* Set the mock PPS */
		set_pps(pps_encoding);

		/* Get L0GPTSZ */
		i = 0;
		l0gptsz = l0gptsz_values[i];

		/* Set the mock L0 page size */
		set_l0gptsz(l0gptsz_encodings[i]);

		/* Get PGS */
		k = 0;
		pgs_encoding = pgs_encodings[k];

		/* Set the mock PGS */
		set_pgs((gpccr_pgs_e)pgs_encoding);

		/* Allocate memory for L0 table */
		l0_mem_size = pps >= l0gptsz ? (pps/l0gptsz) * L0_ENTRY_SIZE : L0_ENTRY_SIZE;

		l0_table = allocate_l0_table_mem(l0_mem_size);

		if (!l0_table) {
			FAIL("Unable to allocate L0 table\n");
		}

		pgs = SIZE_4KB;
		pgs_encoding = GPCCR_PGS_4K;

		num_l0_entries = l0_mem_size / L0_ENTRY_SIZE;

		pas_regions = (pas_region_t *)calloc(PAS_COUNT_MAX, sizeof(pas_region_t));
		if(!pas_regions) {
			FAIL("Could not allocate pas_regions array\n");
		}

		counts = create_pas_regions(pas_regions, pps, pgs, l0gptsz);

		/* Calculate size for L1 tables */
		l1_mem_size = ((l0gptsz / pgs) / 2) * counts.l1_table_count;

		/* Allocate space for L1 tables */
		l1_table = allocate_l1_table_mem(l1_table, l1_mem_size);
		if (!l1_table) {
			FAIL("Unable to allocate L1 table\n");
		}
	}

	TEST_TEARDOWN() {
		if (l0_table) {
			free((void *)l0_table);
		}
		if (l1_table) {
			free((void *)l1_table);
		}
	}

	unsigned long int pps;
	gpccr_pgs_e pgs_encoding;
	size_t pgs;
	gpccr_pps_e pps_encoding;
	unsigned long l0gptsz;
	gpccr_l0gptsz_e l0gptsz_encoding;
	size_t l0_mem_size;
	size_t l1_mem_size;
	uintptr_t l0_table;
	uintptr_t l1_table;
	pas_regions_info_t counts;
	pas_region_t *pas_regions;
};

TEST(gpt_init_pas_l1_tables, init) {

	if (gpt_init_l0_tables(pps_encoding, l0_table,
		l0_mem_size) != 0) {
		printf("Failed to init l0 table for pps %lu, l0gptsz %lu",
			pps, l0gptsz);
		FAIL("\n");
	}
	walk_l0_table(l0_table, l0_mem_size, false);
	if (gpt_init_pas_l1_tables(pgs_encoding, l1_table, l1_mem_size,
		pas_regions, counts.pas_count) != 0) {
		printf("Failed to init l1 table for pps %lu, l0gptsz %lu",
			pps, l0gptsz);
		FAIL("\n");
	}
}

/*
 * TEST_GROUP: misaligned_l1_table
 *  Tests the behavior or wrongly aligned L1 tables
 */
TEST_GROUP(misaligned_l1_table) {
	TEST_SETUP() {
		int i = 0;
		int j = 0;
		int k = 0;

		unsigned int num_l0_entries;
		unsigned int num_l1_tables;
		unsigned int init_pas_count;
		unsigned int l1_table_count;
		unsigned int flags = 0;

		/* Enable MMU and data caches */
		enable_mmu(flags);

		uintptr_t base_pa = 0x0;

		/* Get PPS */
		pps = pps_values[j];
		pps_encoding = pps_encodings[j];

		/* Set the mock PPS */
		set_pps(pps_encoding);

		/* Get L0GPTSZ */
		l0gptsz = l0gptsz_values[i];

		/* Set the mock L0 page size */
		set_l0gptsz(l0gptsz_encodings[i]);

		/* Get PGS */
		pgs_encoding = pgs_encodings[k];

		/* Set the mock PGS */
		set_pgs((gpccr_pgs_e)pgs_encoding);

		/* Allocate memory for L0 table */
		l0_mem_size = pps >= l0gptsz ? (pps/l0gptsz) * L0_ENTRY_SIZE : L0_ENTRY_SIZE;

		l0_table = allocate_l0_table_mem(l0_mem_size);

		if (!l0_table)
			FAIL("Unable to allocate L0 table\n");

		pgs = SIZE_4KB;
		pgs_encoding = GPCCR_PGS_4K;

		num_l0_entries = l0_mem_size / L0_ENTRY_SIZE;

		l1_table_count = pps/l0gptsz;

		/* Calculate size for L1 tables */
		l1_mem_size = ((l0gptsz / pgs) / 2) * l1_table_count;

		/* Allocate space for L1 tables */
		l1_table = allocate_l1_table_mem(l1_table, l1_mem_size);
		if (!l1_table)
			FAIL("Unable to allocate L1 table\n");
	}

	TEST_TEARDOWN() {
		if (l0_table)
			free((void *)l0_table);
		if (l1_table)
			free((void *)l1_table);
	}

	unsigned long int pps;
	gpccr_pgs_e pgs_encoding;
	size_t pgs;
	gpccr_pps_e pps_encoding;
	unsigned long l0gptsz;
	gpccr_l0gptsz_e l0gptsz_encoding;
	size_t l0_mem_size;
	size_t l1_mem_size;
	uintptr_t l0_table;
	uintptr_t l1_table;
};

/*
 * TEST: unaligned
 *  Tests whether a set of misaligned PAS regions can be used to
 *  initialize L1 tables.
 */
TEST(misaligned_l1_table, unaligned){
	pas_region_t pas_regions[] = {
		GPT_MAP_REGION_BLOCK(PAS_1_BASE, PAS_1_SIZE, PAS_1_GPI),
		/* The granule base address is misaligned for this test.
		 * All granule regions are shifted one byte. */
		GPT_MAP_REGION_GRANULE(PAS_2_BASE + 1, PAS_2_SIZE, PAS_2_GPI),
		GPT_MAP_REGION_GRANULE(PAS_3_BASE + 1, PAS_3_SIZE, PAS_3_GPI),
		GPT_MAP_REGION_GRANULE(PAS_4_BASE + 1, PAS_4_SIZE, PAS_4_GPI),
		GPT_MAP_REGION_GRANULE(PAS_5_BASE + 1, PAS_5_SIZE, PAS_5_GPI),
		GPT_MAP_REGION_GRANULE(PAS_6_BASE + 1, PAS_6_SIZE, PAS_6_GPI),
	};
	unsigned int pas_region_count = 6;

	if (gpt_init_l0_tables(pps_encoding, l0_table, l0_mem_size) != 0){
		printf("Failed to init l0 table for pps %lu, l0gptsz %lu", pps, l0gptsz);
		FAIL("\n");
	}
	walk_l0_table(l0_table, l0_mem_size, false);
	if (gpt_init_pas_l1_tables(pgs_encoding, l1_table, l1_mem_size, pas_regions, pas_region_count) == 0){
		printf("Misaligned L1 test failed.\n");
		FAIL("\n");
	}
}
