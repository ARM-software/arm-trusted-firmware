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

#ifndef __BL31_H__
#define __BL31_H__

#include <stdint.h>

/*******************************************************************************
 * Data declarations
 ******************************************************************************/
extern unsigned long bl31_entrypoint;

/******************************************
 * Forward declarations
 *****************************************/
struct meminfo;
struct el_change_info;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
extern void bl31_arch_setup(void);
extern void bl31_next_el_arch_setup(uint32_t security_state);
extern void bl31_set_next_image_type(uint32_t type);
extern uint32_t bl31_get_next_image_type(void);
extern void bl31_prepare_next_image_entry();
extern struct el_change_info *bl31_get_next_image_info(uint32_t type);
extern void bl31_early_platform_setup(bl31_params_t *from_bl2,
					void *plat_params_from_bl2);
extern void bl31_platform_setup(void);
extern void bl31_register_bl32_init(int32_t (*)(void));

#endif /* __BL31_H__ */
