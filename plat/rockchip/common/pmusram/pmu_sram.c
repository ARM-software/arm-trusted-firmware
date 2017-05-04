/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console.h>
#include <debug.h>
#include <platform.h>
#include <plat_private.h>

/*****************************************************************************
 * sram only surpport 32-bits access
 ******************************************************************************/
void u32_align_cpy(uint32_t *dst, const uint32_t *src, size_t bytes)
{
	uint32_t i;

	for (i = 0; i < bytes; i++)
		dst[i] = src[i];
}

void rockchip_plat_sram_mmu_el3(void)
{
#ifdef PLAT_EXTRA_LD_SCRIPT
	size_t sram_size;

	/* sram.text size */
	sram_size = (char *)&__bl31_sram_text_end -
		    (char *)&__bl31_sram_text_start;
	mmap_add_region((unsigned long)&__bl31_sram_text_start,
			(unsigned long)&__bl31_sram_text_start,
			sram_size, MT_MEMORY | MT_RO | MT_SECURE);

	/* sram.data size */
	sram_size = (char *)&__bl31_sram_data_end -
		    (char *)&__bl31_sram_data_start;
	mmap_add_region((unsigned long)&__bl31_sram_data_start,
			(unsigned long)&__bl31_sram_data_start,
			sram_size, MT_MEMORY | MT_RW | MT_SECURE);

	/* sram.incbin size */
	sram_size = (char *)&__sram_incbin_end - (char *)&__sram_incbin_start;
	mmap_add_region((unsigned long)&__sram_incbin_start,
			(unsigned long)&__sram_incbin_start,
			sram_size, MT_NON_CACHEABLE | MT_RW | MT_SECURE);
#else
	/* TODO: Support other SoCs, Just support RK3399 now */
	return;
#endif
}

void plat_rockchip_mem_prepare(void)
{
	/* The code for resuming cpu from suspend must be excuted in pmusram */
	plat_rockchip_pmusram_prepare();
}
