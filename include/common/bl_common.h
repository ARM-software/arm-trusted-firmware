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

#ifndef __BL_COMMON_H__
#define __BL_COMMON_H__

#define SECURE		0
#define NON_SECURE	1

#define UP	1
#define DOWN	0

/*******************************************************************************
 * Constants for loading images. When BLx wants to load BLy, it looks at a
 * meminfo structure to find the extents of free memory. Then depending upon
 * how it has been configured, it can either load BLy at the top or bottom of
 * the free memory. These constants indicate the choice.
 * TODO: Make this configurable while building the trusted firmware.
 ******************************************************************************/
#define TOP_LOAD	0x1
#define BOT_LOAD	!TOP_LOAD
#define LOAD_MASK	(1 << 0)

/******************************************************************************
 * Opcode passed in x0 to tell next EL that we want to run an image.
 * Corresponds to the function ID of the only SMC that the BL1 exception
 * handlers service. That's why the chosen value is the first function ID of
 * the ARM SMC64 range.
 *****************************************************************************/
#define RUN_IMAGE	0xC0000000


#ifndef __ASSEMBLY__

#include <cdefs.h> /* For __dead2 */

/*******************************************************************************
 * Structure used for telling the next BL how much of a particular type of
 * memory is available for its use and how much is already used.
 ******************************************************************************/
typedef struct meminfo {
	unsigned long total_base;
	long total_size;
	unsigned long free_base;
	long free_size;
	unsigned long attr;
	unsigned long next;
} meminfo_t;

typedef struct aapcs64_params {
	unsigned long arg0;
	unsigned long arg1;
	unsigned long arg2;
	unsigned long arg3;
	unsigned long arg4;
	unsigned long arg5;
	unsigned long arg6;
	unsigned long arg7;
} aapcs64_params_t;

/*******************************************************************************
 * This structure represents the superset of information needed while switching
 * exception levels. The only two mechanisms to do so are ERET & SMC. In case of
 * SMC all members apart from 'aapcs64_params' will be ignored.
 ******************************************************************************/
typedef struct el_change_info {
	unsigned long entrypoint;
	unsigned long spsr;
	unsigned long security_state;
	aapcs64_params_t args;
} el_change_info_t;

/*******************************************************************************
 * This structure represents the superset of information that can be passed to
 * BL31 e.g. while passing control to it from BL2. The BL32 parameters will be
 * populated only if BL2 detects its presence.
 ******************************************************************************/
typedef struct bl31_args {
	meminfo_t bl31_meminfo;
	el_change_info_t bl32_image_info;
	meminfo_t bl32_meminfo;
	el_change_info_t bl33_image_info;
	meminfo_t bl33_meminfo;
} bl31_args_t;

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
unsigned long page_align(unsigned long, unsigned);
void change_security_state(unsigned int);
void __dead2 drop_el(aapcs64_params_t *, unsigned long, unsigned long);
void __dead2 raise_el(aapcs64_params_t *);
void __dead2 change_el(el_change_info_t *);
unsigned long make_spsr(unsigned long, unsigned long, unsigned long);
void init_bl2_mem_layout(meminfo_t *,
			meminfo_t *,
			unsigned int,
			unsigned long) __attribute__((weak));
void init_bl31_mem_layout(const meminfo_t *,
			meminfo_t *,
			unsigned int) __attribute__((weak));
unsigned long image_size(const char *);
unsigned long load_image(meminfo_t *,
			const char *,
			unsigned int,
			unsigned long);
void __dead2 run_image(unsigned long entrypoint,
			unsigned long spsr,
			unsigned long security_state,
			void *first_arg,
			void *second_arg);
unsigned long *get_el_change_mem_ptr(void);
extern const char build_message[];

#endif /*__ASSEMBLY__*/

#endif /* __BL_COMMON_H__ */
