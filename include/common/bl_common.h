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

#define SECURE		0x0
#define NON_SECURE	0x1
#define sec_state_is_valid(s) (((s) == SECURE) || ((s) == NON_SECURE))

#define UP	1
#define DOWN	0

/*******************************************************************************
 * Constants to identify the location of a memory region in a given memory
 * layout.
******************************************************************************/
#define TOP	0x1
#define BOTTOM	!TOP

/******************************************************************************
 * Opcode passed in x0 to tell next EL that we want to run an image.
 * Corresponds to the function ID of the only SMC that the BL1 exception
 * handlers service. That's why the chosen value is the first function ID of
 * the ARM SMC64 range.
 *****************************************************************************/
#define RUN_IMAGE	0xC0000000

/*******************************************************************************
 * Constants that allow assembler code to access members of and the
 * 'entry_point_info' structure at their correct offsets.
 ******************************************************************************/
#define ENTRY_POINT_INFO_PC_OFFSET	0x08
#define ENTRY_POINT_INFO_ARGS_OFFSET	0x18

#define PARAM_EP_SECURITY_MASK    0x1
#define GET_SECURITY_STATE(x) (x & PARAM_EP_SECURITY_MASK)
#define SET_SECURITY_STATE(x, security) \
			((x) = ((x) & ~PARAM_EP_SECURITY_MASK) | (security))

#define EP_EE_MASK	0x2
#define EP_EE_LITTLE	0x0
#define EP_EE_BIG	0x2
#define EP_GET_EE(x) (x & EP_EE_MASK)
#define EP_SET_EE(x, ee) ((x) = ((x) & ~EP_EE_MASK) | (ee))

#define EP_ST_MASK	0x4
#define EP_ST_DISABLE	0x0
#define EP_ST_ENABLE	0x4
#define EP_GET_ST(x) (x & EP_ST_MASK)
#define EP_SET_ST(x, ee) ((x) = ((x) & ~EP_ST_MASK) | (ee))

#define PARAM_EP     0x01
#define PARAM_IMAGE_BINARY  0x02
#define PARAM_BL31       0x03

#define VERSION_1		0x01

#define SET_PARAM_HEAD(_p, _type, _ver, _attr) do { \
	(_p)->h.type = (uint8_t)(_type); \
	(_p)->h.version = (uint8_t)(_ver); \
	(_p)->h.size = (uint16_t)sizeof(*_p); \
	(_p)->h.attr = (uint32_t)(_attr) ; \
	} while (0)

#ifndef __ASSEMBLY__
#include <cdefs.h> /* For __dead2 */
#include <cassert.h>
#include <stdint.h>
#include <stddef.h>

/*******************************************************************************
 * Structure used for telling the next BL how much of a particular type of
 * memory is available for its use and how much is already used.
 ******************************************************************************/
typedef struct meminfo {
	uint64_t total_base;
	size_t total_size;
	uint64_t free_base;
	size_t free_size;
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

/***************************************************************************
 * This structure provides version information and the size of the
 * structure, attributes for the structure it represents
 ***************************************************************************/
typedef struct param_header {
	uint8_t type;		/* type of the structure */
	uint8_t version;    /* version of this structure */
	uint16_t size;      /* size of this structure in bytes */
	uint32_t attr;      /* attributes: unused bits SBZ */
} param_header_t;

/*****************************************************************************
 * This structure represents the superset of information needed while
 * switching exception levels. The only two mechanisms to do so are
 * ERET & SMC. Security state is indicated using bit zero of header
 * attribute
 * NOTE: BL1 expects entrypoint followed by spsr while processing
 * SMC to jump to BL31 from the start of entry_point_info
 *****************************************************************************/
typedef struct entry_point_info {
	param_header_t h;
	uintptr_t pc;
	uint32_t spsr;
	aapcs64_params_t args;
} entry_point_info_t;

/*****************************************************************************
 * Image info binary provides information from the image loader that
 * can be used by the firmware to manage available trusted RAM.
 * More advanced firmware image formats can provide additional
 * information that enables optimization or greater flexibility in the
 * common firmware code
 *****************************************************************************/
typedef struct image_info {
	param_header_t h;
	uintptr_t image_base;   /* physical address of base of image */
	uint32_t image_size;    /* bytes read from image file */
} image_info_t;

/*******************************************************************************
 * This structure represents the superset of information that can be passed to
 * BL31 e.g. while passing control to it from BL2. The BL32 parameters will be
 * populated only if BL2 detects its presence. A pointer to a structure of this
 * type should be passed in X3 to BL31's cold boot entrypoint
 *
 * Use of this structure and the X3 parameter is not mandatory: the BL3-1
 * platform code can use other mechanisms to provide the necessary information
 * about BL3-2 and BL3-3 to the common and SPD code.
 *
 * BL3-1 image information is mandatory if this structure is used. If either of
 * the optional BL3-2 and BL3-3 image information is not provided, this is
 * indicated by the respective image_info pointers being zero.
 ******************************************************************************/
typedef struct bl31_params {
	param_header_t h;
	image_info_t *bl31_image_info;
	entry_point_info_t *bl32_ep_info;
	image_info_t *bl32_image_info;
	entry_point_info_t *bl33_ep_info;
	image_info_t *bl33_image_info;
} bl31_params_t;


/*
 * Compile time assertions related to the 'entry_point_info' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(ENTRY_POINT_INFO_PC_OFFSET ==
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_BL31_pc_offset_mismatch);

CASSERT(ENTRY_POINT_INFO_ARGS_OFFSET == \
		__builtin_offsetof(entry_point_info_t, args), \
		assert_BL31_args_offset_mismatch);

CASSERT(sizeof(unsigned long) ==
		__builtin_offsetof(entry_point_info_t, spsr) - \
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_entrypoint_and_spsr_should_be_adjacent);

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
unsigned long page_align(unsigned long, unsigned);
void change_security_state(unsigned int);
unsigned long image_size(const char *);
int load_image(meminfo_t *mem_layout,
	       const char *image_name,
	       uint64_t image_base,
	       image_info_t *image_data,
	       entry_point_info_t *entry_point_info);
extern const char build_message[];
extern const char version_string[];

void reserve_mem(uint64_t *free_base, size_t *free_size,
		uint64_t addr, size_t size);

#endif /*__ASSEMBLY__*/

#endif /* __BL_COMMON_H__ */
