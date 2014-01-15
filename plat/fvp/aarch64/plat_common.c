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
/* Included only for error codes */
#include <psci.h>

unsigned char platform_normal_stacks[PLATFORM_STACK_SIZE][PLATFORM_CORE_COUNT]
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_normal_stacks")));

/*******************************************************************************
 * This array holds the characteristics of the differences between the three
 * FVP platforms (Base, A53_A57 & Foundation). It will be populated during cold
 * boot at each boot stage by the primary before enabling the MMU (to allow cci
 * configuration) & used thereafter. Each BL will have its own copy to allow
 * independent operation.
 ******************************************************************************/
static unsigned long platform_config[CONFIG_LIMIT];

/*******************************************************************************
 * TODO: Check page table alignment to avoid space wastage
 ******************************************************************************/

/*******************************************************************************
 * Level 1 translation tables need 4 entries for the 4GB address space accessib-
 * le by the secure firmware. Input address space will be restricted using the
 * T0SZ settings in the TCR.
 ******************************************************************************/
static unsigned long l1_xlation_table[ADDR_SPACE_SIZE >> 30]
__attribute__ ((aligned((ADDR_SPACE_SIZE >> 30) << 3)));

/*******************************************************************************
 * Level 2 translation tables describe the first & second gb of the address
 * space needed to address secure peripherals e.g. trusted ROM and RAM.
 ******************************************************************************/
static unsigned long l2_xlation_table[NUM_L2_PAGETABLES][NUM_2MB_IN_GB]
__attribute__ ((aligned(NUM_2MB_IN_GB << 3)));

/*******************************************************************************
 * Level 3 translation tables (2 sets) describe the trusted & non-trusted RAM
 * regions at a granularity of 4K.
 ******************************************************************************/
static unsigned long l3_xlation_table[NUM_L3_PAGETABLES][NUM_4K_IN_2MB]
__attribute__ ((aligned(NUM_4K_IN_2MB << 3)));

/*******************************************************************************
 * Helper to create a level 1/2 table descriptor which points to a level 2/3
 * table.
 ******************************************************************************/
static unsigned long create_table_desc(unsigned long *next_table_ptr)
{
	unsigned long desc = (unsigned long) next_table_ptr;

	/* Clear the last 12 bits */
	desc >>= FOUR_KB_SHIFT;
	desc <<= FOUR_KB_SHIFT;

	desc |= TABLE_DESC;

	return desc;
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to addr
 ******************************************************************************/
static unsigned long create_block_desc(unsigned long desc,
				       unsigned long addr,
				       unsigned int level)
{
	switch (level) {
	case LEVEL1:
		desc |= (addr << FIRST_LEVEL_DESC_N) | BLOCK_DESC;
		break;
	case LEVEL2:
		desc |= (addr << SECOND_LEVEL_DESC_N) | BLOCK_DESC;
		break;
	case LEVEL3:
		desc |= (addr << THIRD_LEVEL_DESC_N) | TABLE_DESC;
		break;
	default:
		assert(0);
	}

	return desc;
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to output_
 * addr with Device nGnRE attributes.
 ******************************************************************************/
static unsigned long create_device_block(unsigned long output_addr,
					 unsigned int level,
					 unsigned int ns)
{
	unsigned long upper_attrs, lower_attrs, desc;

	lower_attrs = LOWER_ATTRS(ACCESS_FLAG | OSH | AP_RW);
	lower_attrs |= LOWER_ATTRS(ns | ATTR_DEVICE_INDEX);
	upper_attrs = UPPER_ATTRS(XN);
	desc = upper_attrs | lower_attrs;

	return create_block_desc(desc, output_addr, level);
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to output_
 * addr with inner-shareable normal wbwa read-only memory attributes.
 ******************************************************************************/
static unsigned long create_romem_block(unsigned long output_addr,
					unsigned int level,
					unsigned int ns)
{
	unsigned long upper_attrs, lower_attrs, desc;

	lower_attrs = LOWER_ATTRS(ACCESS_FLAG | ISH | AP_RO);
	lower_attrs |= LOWER_ATTRS(ns | ATTR_IWBWA_OWBWA_NTR_INDEX);
	upper_attrs = UPPER_ATTRS(0ull);
	desc = upper_attrs | lower_attrs;

	return create_block_desc(desc, output_addr, level);
}

/*******************************************************************************
 * Helper to create a level 1/2/3 block descriptor which maps the va to output_
 * addr with inner-shareable normal wbwa read-write memory attributes.
 ******************************************************************************/
static unsigned long create_rwmem_block(unsigned long output_addr,
					unsigned int level,
					unsigned int ns)
{
	unsigned long upper_attrs, lower_attrs, desc;

	lower_attrs = LOWER_ATTRS(ACCESS_FLAG | ISH | AP_RW);
	lower_attrs |= LOWER_ATTRS(ns | ATTR_IWBWA_OWBWA_NTR_INDEX);
	upper_attrs = UPPER_ATTRS(XN);
	desc = upper_attrs | lower_attrs;

	return create_block_desc(desc, output_addr, level);
}

/*******************************************************************************
 * Create page tables as per the platform memory map. Certain aspects of page
 * talble creating have been abstracted in the above routines. This can be impr-
 * oved further.
 * TODO: Move the page table setup helpers into the arch or lib directory
 *******************************************************************************/
static unsigned long fill_xlation_tables(meminfo *tzram_layout,
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
	for (idx = 0; idx < (ADDR_SPACE_SIZE >> 30); idx++) {

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

/*******************************************************************************
 * Enable the MMU assuming that the pagetables have already been created
 *******************************************************************************/
void enable_mmu()
{
	unsigned long mair, tcr, ttbr, sctlr;
	unsigned long current_el = read_current_el();

	/* Set the attributes in the right indices of the MAIR */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
				  ATTR_IWBWA_OWBWA_NTR_INDEX);
	write_mair(mair);

	/*
	 * Set TCR bits as well. Inner & outer WBWA & shareable + T0SZ = 32
	 */
	tcr = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA |
		  TCR_RGN_INNER_WBA | TCR_T0SZ_4GB;
	if (GET_EL(current_el) == MODE_EL3) {
		tcr |= TCR_EL3_RES1;
		/* Invalidate EL3 TLBs */
		tlbialle3();
	} else {
		/* Invalidate EL1 TLBs */
		tlbivmalle1();
	}

	write_tcr(tcr);

	/* Set TTBR bits as well */
	assert(((unsigned long)l1_xlation_table & (sizeof(l1_xlation_table) - 1)) == 0);
	ttbr = (unsigned long) l1_xlation_table;
	write_ttbr0(ttbr);

	sctlr = read_sctlr();
	sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT | SCTLR_I_BIT;
	sctlr |= SCTLR_A_BIT | SCTLR_C_BIT;
	write_sctlr(sctlr);

	return;
}

void disable_mmu(void)
{
	/* Zero out the MMU related registers */
	write_mair(0);
	write_tcr(0);
	write_ttbr0(0);
	write_sctlr(0);

	/* Flush the caches */
	dcsw_op_all(DCCISW);

	return;
}

/*******************************************************************************
 * Setup the pagetables as per the platform memory map & initialize the mmu
 *******************************************************************************/
void configure_mmu(meminfo *mem_layout,
		   unsigned long ro_start,
		   unsigned long ro_limit,
		   unsigned long coh_start,
		   unsigned long coh_limit)
{
	assert(IS_PAGE_ALIGNED(ro_start));
	assert(IS_PAGE_ALIGNED(ro_limit));
	assert(IS_PAGE_ALIGNED(coh_start));
	assert(IS_PAGE_ALIGNED(coh_limit));

	fill_xlation_tables(mem_layout,
			    ro_start,
			    ro_limit,
			    coh_start,
			    coh_limit);
	enable_mmu();
	return;
}

/* Simple routine which returns a configuration variable value */
unsigned long platform_get_cfgvar(unsigned int var_id)
{
	assert(var_id < CONFIG_LIMIT);
	return platform_config[var_id];
}

/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation FVP
 * models and the two flavours of the Base FVP models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
int platform_config_setup(void)
{
	unsigned int rev, hbi, bld, arch, sys_id, midr_pn;

	sys_id = mmio_read_32(VE_SYSREGS_BASE + V2M_SYS_ID);
	rev = (sys_id >> SYS_ID_REV_SHIFT) & SYS_ID_REV_MASK;
	hbi = (sys_id >> SYS_ID_HBI_SHIFT) & SYS_ID_HBI_MASK;
	bld = (sys_id >> SYS_ID_BLD_SHIFT) & SYS_ID_BLD_MASK;
	arch = (sys_id >> SYS_ID_ARCH_SHIFT) & SYS_ID_ARCH_MASK;

	assert(rev == REV_FVP);
	assert(arch == ARCH_MODEL);

	/*
	 * The build field in the SYS_ID tells which variant of the GIC
	 * memory is implemented by the model.
	 */
	switch (bld) {
	case BLD_GIC_VE_MMAP:
		platform_config[CONFIG_GICD_ADDR] = VE_GICD_BASE;
		platform_config[CONFIG_GICC_ADDR] = VE_GICC_BASE;
		platform_config[CONFIG_GICH_ADDR] = VE_GICH_BASE;
		platform_config[CONFIG_GICV_ADDR] = VE_GICV_BASE;
		break;
	case BLD_GIC_A53A57_MMAP:
		platform_config[CONFIG_GICD_ADDR] = BASE_GICD_BASE;
		platform_config[CONFIG_GICC_ADDR] = BASE_GICC_BASE;
		platform_config[CONFIG_GICH_ADDR] = BASE_GICH_BASE;
		platform_config[CONFIG_GICV_ADDR] = BASE_GICV_BASE;
		break;
	default:
		assert(0);
	}

	/*
	 * The hbi field in the SYS_ID is 0x020 for the Base FVP & 0x010
	 * for the Foundation FVP.
	 */
	switch (hbi) {
	case HBI_FOUNDATION:
		platform_config[CONFIG_MAX_AFF0] = 4;
		platform_config[CONFIG_MAX_AFF1] = 1;
		platform_config[CONFIG_CPU_SETUP] = 0;
		platform_config[CONFIG_BASE_MMAP] = 0;
		platform_config[CONFIG_HAS_CCI] = 0;
		break;
	case HBI_FVP_BASE:
		midr_pn = (read_midr() >> MIDR_PN_SHIFT) & MIDR_PN_MASK;
		if ((midr_pn == MIDR_PN_A57) || (midr_pn == MIDR_PN_A53))
			platform_config[CONFIG_CPU_SETUP] = 1;
		else
			platform_config[CONFIG_CPU_SETUP] = 0;

		platform_config[CONFIG_MAX_AFF0] = 4;
		platform_config[CONFIG_MAX_AFF1] = 2;
		platform_config[CONFIG_BASE_MMAP] = 1;
		platform_config[CONFIG_HAS_CCI] = 1;
		break;
	default:
		assert(0);
	}

	return 0;
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return NS_IMAGE_OFFSET;
}
