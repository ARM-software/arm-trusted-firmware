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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <mmio.h>
#include <platform.h>
#include <xlat_tables.h>


void enable_mmu()
{
	unsigned long mair, tcr, ttbr, sctlr;
	unsigned long current_el = read_current_el();

	/* Set the attributes in the right indices of the MAIR */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
				  ATTR_IWBWA_OWBWA_NTR_INDEX);

	/*
	 * Set TCR bits as well. Inner & outer WBWA & shareable + T0SZ = 32
	 */
	tcr = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA |
		  TCR_RGN_INNER_WBA | TCR_T0SZ_4GB;

	/* Set TTBR bits as well */
	ttbr = (unsigned long) l1_xlation_table;

	if (GET_EL(current_el) == MODE_EL3) {
		assert((read_sctlr_el3() & SCTLR_M_BIT) == 0);

		write_mair_el3(mair);
		tcr |= TCR_EL3_RES1;
		/* Invalidate EL3 TLBs */
		tlbialle3();

		write_tcr_el3(tcr);
		write_ttbr0_el3(ttbr);

		/* ensure all translation table writes have drained into memory,
		 * the TLB invalidation is complete, and translation register
		 * writes are committed before enabling the MMU
		 */
		dsb();
		isb();

		sctlr = read_sctlr_el3();
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT | SCTLR_I_BIT;
		sctlr |= SCTLR_A_BIT | SCTLR_C_BIT;
		write_sctlr_el3(sctlr);
	} else {
		assert((read_sctlr_el1() & SCTLR_M_BIT) == 0);

		write_mair_el1(mair);
		/* Invalidate EL1 TLBs */
		tlbivmalle1();

		write_tcr_el1(tcr);
		write_ttbr0_el1(ttbr);

		/* ensure all translation table writes have drained into memory,
		 * the TLB invalidation is complete, and translation register
		 * writes are committed before enabling the MMU
		 */
		dsb();
		isb();

		sctlr = read_sctlr_el1();
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT | SCTLR_I_BIT;
		sctlr |= SCTLR_A_BIT | SCTLR_C_BIT;
		write_sctlr_el1(sctlr);
	}
	/* ensure the MMU enable takes effect immediately */
	isb();

	return;
}

static const mmap_region_t juno_mmap[] = {
	{ TZROM_BASE,		TZROM_SIZE,		MT_MEMORY | MT_RO | MT_SECURE },
	{ MHU_SECURE_BASE,	MHU_SECURE_SIZE,	(MHU_PAYLOAD_CACHED ? MT_MEMORY : MT_DEVICE)
								  | MT_RW | MT_SECURE },
//	{ TZRAM_BASE,		TZRAM_SIZE,		MT_MEMORY | MT_RW | MT_SECURE },  /* configure_mmu() meminfo arg sets subset of this */
	{ FLASH_BASE,		FLASH_SIZE,		MT_MEMORY | MT_RO | MT_SECURE },
	{ EMMC_BASE,		EMMC_SIZE,		MT_MEMORY | MT_RO | MT_SECURE },
	{ PSRAM_BASE,		PSRAM_SIZE,		MT_MEMORY | MT_RW | MT_SECURE }, /* Used for 'TZDRAM' */
	{ IOFPGA_BASE,		IOFPGA_SIZE,		MT_DEVICE | MT_RW | MT_SECURE },
//	{ NSROM_BASE,		NSROM_SIZE,		MT_MEMORY | MT_RW | MT_NS },	 /* Eats a page table so leave it out for now */
	{ DEVICE0_BASE,		DEVICE0_SIZE,		MT_DEVICE | MT_RW | MT_SECURE },
	{ NSRAM_BASE,		NSRAM_SIZE,		MT_MEMORY | MT_RW | MT_NS },
	{ DEVICE1_BASE,		DEVICE1_SIZE,		MT_DEVICE | MT_RW | MT_SECURE },
	{ DRAM_BASE,		DRAM_SIZE,		MT_MEMORY | MT_RW | MT_NS },
	{0}
};

void configure_mmu(meminfo_t *mem_layout,
		   unsigned long ro_start,
		   unsigned long ro_limit,
		   unsigned long coh_start,
		   unsigned long coh_limit)
{
	mmap_add_region(mem_layout->total_base, mem_layout->total_size,
				MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_limit - ro_start,
				MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(coh_start, coh_limit - coh_start,
				MT_DEVICE | MT_RW | MT_SECURE);

	mmap_add(juno_mmap);

	init_xlat_tables();

	enable_mmu();
	return;
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return NS_IMAGE_OFFSET;
}

uint64_t plat_get_syscnt_freq(void)
{
	uint64_t counter_base_frequency;

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	assert(counter_base_frequency != 0);

	return counter_base_frequency;
}
