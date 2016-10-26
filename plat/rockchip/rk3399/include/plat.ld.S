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
#ifndef __ROCKCHIP_PLAT_LD_S__
#define __ROCKCHIP_PLAT_LD_S__

MEMORY {
    SRAM (rwx): ORIGIN = SRAM_BASE, LENGTH = SRAM_SIZE
}

SECTIONS
{
	. = SRAM_BASE;
	ASSERT(. == ALIGN(4096),
		"SRAM_BASE address is not aligned on a page boundary.")

	/*
	 * The SRAM space allocation for RK3399
	 * ----------------
	 * | m0 code bin
	 * ----------------
	 * | sram text
	 * ----------------
	 * | sram data
	 * ----------------
	 */
	.incbin_sram : ALIGN(4096) {
		__sram_incbin_start = .;
		*(.sram.incbin)
		. = ALIGN(4096);
		__sram_incbin_end = .;
	} >SRAM

	.text_sram : ALIGN(4096) {
		__bl31_sram_text_start = .;
		*(.sram.text)
		*(.sram.rodata)
		. = ALIGN(4096);
		__bl31_sram_text_end = .;
	} >SRAM

	.data_sram : ALIGN(4096) {
		__bl31_sram_data_start = .;
		*(.sram.data)
		. = ALIGN(4096);
		__bl31_sram_data_end = .;
	} >SRAM
}

#endif /* __ROCKCHIP_PLAT_LD_S__ */
