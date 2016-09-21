/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __BL1_PRIVATE_H__
#define __BL1_PRIVATE_H__

#include <types.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will tell us where BL1 lives
 * in Trusted ROM and RAM
 ******************************************************************************/
extern uintptr_t __BL1_ROM_END__;
#define BL1_ROM_END (uintptr_t)(&__BL1_ROM_END__)

extern uintptr_t __BL1_RAM_START__;
extern uintptr_t __BL1_RAM_END__;
#define BL1_RAM_BASE (uintptr_t)(&__BL1_RAM_START__)
#define BL1_RAM_LIMIT (uintptr_t)(&__BL1_RAM_END__)

/******************************************
 * Function prototypes
 *****************************************/
void bl1_arch_setup(void);
void bl1_arch_next_el_setup(void);

void bl1_prepare_next_image(unsigned int image_id);

register_t bl1_fwu_smc_handler(unsigned int smc_fid,
		register_t x1,
		register_t x2,
		register_t x3,
		register_t x4,
		void *cookie,
		void *handle,
		unsigned int flags);
#endif /* __BL1_PRIVATE_H__ */
