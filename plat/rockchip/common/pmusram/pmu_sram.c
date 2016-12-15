/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
