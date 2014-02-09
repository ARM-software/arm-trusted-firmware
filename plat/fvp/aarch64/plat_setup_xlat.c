/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <platform.h>
#include <bl_common.h>

/*******************************************************************************
 * TODO: Check page table alignment to avoid space wastage
 ******************************************************************************/

/*******************************************************************************
 * Level 1 translation tables need 4 entries for the 4GB address space accessib-
 * le by the secure firmware. Input address space will be restricted using the
 * T0SZ settings in the TCR.
 ******************************************************************************/
static unsigned long l1_xlation_table[NUM_GB_IN_4GB]
__attribute__ ((aligned((NUM_GB_IN_4GB) << 3)));

/*******************************************************************************
 * Level 2 translation tables describe the first & second gb of the address
 * space needed to address secure peripherals e.g. trusted ROM and RAM.
 ******************************************************************************/
static unsigned long l2_xlation_table[NUM_L2_PAGETABLES][NUM_2MB_IN_GB]
__attribute__ ((aligned(NUM_2MB_IN_GB << 3),
	section("xlat_table")));

/*******************************************************************************
 * Level 3 translation tables (2 sets) describe the trusted & non-trusted RAM
 * regions at a granularity of 4K.
 ******************************************************************************/
static unsigned long l3_xlation_table[NUM_L3_PAGETABLES][NUM_4K_IN_2MB]
__attribute__ ((aligned(NUM_4K_IN_2MB << 3),
	section("xlat_table")));

/*******************************************************************************
 * Create page tables as per the platform memory map. Certain aspects of page
 * talble creating have been abstracted in the above routines. This can be impr-
 * oved further.
 * TODO: Move the page table setup helpers into the arch or lib directory
 *******************************************************************************/
unsigned long fill_xlation_tables(meminfo *tzram_layout,
				  unsigned long ro_start,
				  unsigned long ro_limit,
				  unsigned long coh_start,
				  unsigned long coh_limit)
{
	unsigned long l2_desc, l3_desc;
	unsigned long *xt_addr = 0, *pt_addr, off = 0;
	unsigned long trom_start_index, trom_end_index;
	unsigned long tzram_start_index, tzram_end_index;
	unsigned long flash0_start_index, flash0_end_index;
	unsigned long flash1_start_index, flash1_end_index;
	unsigned long vram_start_index, vram_end_index;
	unsigned long nsram_start_index, nsram_end_index;
	unsigned long tdram_start_index, tdram_end_index;
	unsigned long dram_start_index, dram_end_index;
	unsigned long dev0_start_index, dev0_end_index;
	unsigned long dev1_start_index, dev1_end_index;
	unsigned int idx;

	/*****************************************************************
	 * LEVEL1 PAGETABLE SETUP
	 *
	 * Find the start and end indices of the memory peripherals in the
	 * first level pagetables. These are the main areas we care about.
	 * Also bump the end index by one if its equal to the start to
	 * allow for regions which lie completely in a GB.
	 *****************************************************************/
	trom_start_index = ONE_GB_INDEX(TZROM_BASE);
	dev0_start_index = ONE_GB_INDEX(TZRNG_BASE);
	dram_start_index = ONE_GB_INDEX(DRAM_BASE);
	dram_end_index = ONE_GB_INDEX(DRAM_BASE + DRAM_SIZE);

	if (dram_end_index == dram_start_index)
		dram_end_index++;

	/*
	 * Fill up the level1 translation table first
	 */
	for (idx = 0; idx < NUM_GB_IN_4GB; idx++) {

		/*
		 * Fill up the entry for the TZROM. This will cover
		 * everything in the first GB.
		 */
		if (idx == trom_start_index) {
			xt_addr = &l2_xlation_table[GB1_L2_PAGETABLE][0];
			l1_xlation_table[idx] = create_table_desc(xt_addr);
			continue;
		}

		/*
		 * Mark the second gb as device
		 */
		if (idx == dev0_start_index) {
			xt_addr = &l2_xlation_table[GB2_L2_PAGETABLE][0];
			l1_xlation_table[idx] = create_table_desc(xt_addr);
			continue;
		}

		/*
		 * Fill up the block entry for the DRAM with Normal
		 * inner-WBWA outer-WBWA non-transient attributes.
		 * This will cover 2-4GB. Note that the acesses are
		 * marked as non-secure.
		 */
		if ((idx >= dram_start_index) && (idx < dram_end_index)) {
			l1_xlation_table[idx] = create_rwmem_block(idx, LEVEL1,
								   NS);
			continue;
		}

		assert(0);
	}


	/*****************************************************************
	 * LEVEL2 PAGETABLE SETUP
	 *
	 * Find the start and end indices of the memory & peripherals in the
	 * second level pagetables.
	 ******************************************************************/

	/* Initializations for the 1st GB */
	trom_start_index = TWO_MB_INDEX(TZROM_BASE);
	trom_end_index = TWO_MB_INDEX(TZROM_BASE + TZROM_SIZE);
	if (trom_end_index == trom_start_index)
		trom_end_index++;

	tdram_start_index = TWO_MB_INDEX(TZDRAM_BASE);
	tdram_end_index = TWO_MB_INDEX(TZDRAM_BASE + TZDRAM_SIZE);
	if (tdram_end_index == tdram_start_index)
		tdram_end_index++;

	flash0_start_index = TWO_MB_INDEX(FLASH0_BASE);
	flash0_end_index = TWO_MB_INDEX(FLASH0_BASE + TZROM_SIZE);
	if (flash0_end_index == flash0_start_index)
		flash0_end_index++;

	flash1_start_index = TWO_MB_INDEX(FLASH1_BASE);
	flash1_end_index = TWO_MB_INDEX(FLASH1_BASE + FLASH1_SIZE);
	if (flash1_end_index == flash1_start_index)
		flash1_end_index++;

	vram_start_index = TWO_MB_INDEX(VRAM_BASE);
	vram_end_index = TWO_MB_INDEX(VRAM_BASE + VRAM_SIZE);
	if (vram_end_index == vram_start_index)
		vram_end_index++;

	dev0_start_index = TWO_MB_INDEX(DEVICE0_BASE);
	dev0_end_index = TWO_MB_INDEX(DEVICE0_BASE + DEVICE0_SIZE);
	if (dev0_end_index == dev0_start_index)
		dev0_end_index++;

	dev1_start_index = TWO_MB_INDEX(DEVICE1_BASE);
	dev1_end_index = TWO_MB_INDEX(DEVICE1_BASE + DEVICE1_SIZE);
	if (dev1_end_index == dev1_start_index)
		dev1_end_index++;

	/* Since the size is < 2M this is a single index */
	tzram_start_index = TWO_MB_INDEX(tzram_layout->total_base);
	nsram_start_index = TWO_MB_INDEX(NSRAM_BASE);

	/*
	 * Fill up the level2 translation table for the first GB next
	 */
	for (idx = 0; idx < NUM_2MB_IN_GB; idx++) {

		l2_desc = INVALID_DESC;
		xt_addr = &l2_xlation_table[GB1_L2_PAGETABLE][idx];

		/* Block entries for 64M of trusted Boot ROM */
		if ((idx >= trom_start_index) && (idx < trom_end_index))
			l2_desc = create_romem_block(idx, LEVEL2, 0);

		/* Single L3 page table entry for 256K of TZRAM */
		if (idx == tzram_start_index) {
			pt_addr = &l3_xlation_table[TZRAM_PAGETABLE][0];
			l2_desc = create_table_desc(pt_addr);
		}

		/* Block entries for 32M of trusted DRAM */
		if ((idx >= tdram_start_index) && (idx <= tdram_end_index))
			l2_desc = create_rwmem_block(idx, LEVEL2, 0);

		/* Block entries for 64M of aliased trusted Boot ROM */
		if ((idx >= flash0_start_index) && (idx < flash0_end_index))
			l2_desc = create_romem_block(idx, LEVEL2, 0);

		/* Block entries for 64M of flash1 */
		if ((idx >= flash1_start_index) && (idx < flash1_end_index))
			l2_desc = create_romem_block(idx, LEVEL2, 0);

		/* Block entries for 32M of VRAM */
		if ((idx >= vram_start_index) && (idx < vram_end_index))
			l2_desc = create_rwmem_block(idx, LEVEL2, 0);

		/* Block entries for all the devices in the first gb */
		if ((idx >= dev0_start_index) && (idx < dev0_end_index))
			l2_desc = create_device_block(idx, LEVEL2, 0);

		/* Block entries for all the devices in the first gb */
		if ((idx >= dev1_start_index) && (idx < dev1_end_index))
			l2_desc = create_device_block(idx, LEVEL2, 0);

		/* Single L3 page table entry for 64K of NSRAM */
		if (idx == nsram_start_index) {
			pt_addr = &l3_xlation_table[NSRAM_PAGETABLE][0];
			l2_desc = create_table_desc(pt_addr);
		}

		*xt_addr = l2_desc;
	}


	/*
	 * Initializations for the 2nd GB. Mark everything as device
	 * for the time being as the memory map is not final. Each
	 * index will need to be offset'ed to allow absolute values
	 */
	off = NUM_2MB_IN_GB;
	for (idx = off; idx < (NUM_2MB_IN_GB + off); idx++) {
		l2_desc = create_device_block(idx, LEVEL2, 0);
		xt_addr = &l2_xlation_table[GB2_L2_PAGETABLE][idx - off];
		*xt_addr = l2_desc;
	}


	/*****************************************************************
	 * LEVEL3 PAGETABLE SETUP
	 *****************************************************************/

	/* Fill up the level3 pagetable for the trusted SRAM. */
	tzram_start_index = FOUR_KB_INDEX(tzram_layout->total_base);
	tzram_end_index = FOUR_KB_INDEX(tzram_layout->total_base +
					tzram_layout->total_size);
	if (tzram_end_index == tzram_start_index)
		tzram_end_index++;

	/* Reusing trom* to mark RO memory. */
	trom_start_index = FOUR_KB_INDEX(ro_start);
	trom_end_index = FOUR_KB_INDEX(ro_limit);
	if (trom_end_index == trom_start_index)
		trom_end_index++;

	/* Reusing dev* to mark coherent device memory. */
	dev0_start_index = FOUR_KB_INDEX(coh_start);
	dev0_end_index = FOUR_KB_INDEX(coh_limit);
	if (dev0_end_index == dev0_start_index)
		dev0_end_index++;


	/* Each index will need to be offset'ed to allow absolute values */
	off = FOUR_KB_INDEX(TZRAM_BASE);
	for (idx = off; idx < (NUM_4K_IN_2MB + off); idx++) {

		l3_desc = INVALID_DESC;
		xt_addr = &l3_xlation_table[TZRAM_PAGETABLE][idx - off];

		if (idx >= tzram_start_index && idx < tzram_end_index)
			l3_desc = create_rwmem_block(idx, LEVEL3, 0);

		if (idx >= trom_start_index && idx < trom_end_index)
			l3_desc = create_romem_block(idx, LEVEL3, 0);

		if (idx >= dev0_start_index && idx < dev0_end_index)
			l3_desc = create_device_block(idx, LEVEL3, 0);

		*xt_addr = l3_desc;
	}

	/* Fill up the level3 pagetable for the non-trusted SRAM. */
	nsram_start_index = FOUR_KB_INDEX(NSRAM_BASE);
	nsram_end_index = FOUR_KB_INDEX(NSRAM_BASE + NSRAM_SIZE);
	if (nsram_end_index == nsram_start_index)
		nsram_end_index++;

	 /* Each index will need to be offset'ed to allow absolute values */
	off = FOUR_KB_INDEX(NSRAM_BASE);
	for (idx = off; idx < (NUM_4K_IN_2MB + off); idx++) {

		l3_desc = INVALID_DESC;
		xt_addr = &l3_xlation_table[NSRAM_PAGETABLE][idx - off];

		if (idx >= nsram_start_index && idx < nsram_end_index)
			l3_desc = create_rwmem_block(idx, LEVEL3, NS);

		*xt_addr = l3_desc;
	}

	return (unsigned long) l1_xlation_table;
}
