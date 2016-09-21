/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#include <bl_common.h>
#include <platform_def.h>

/******************************************************************************
 * Required platform porting definitions that are expected to be common to
 * all platforms
 *****************************************************************************/

/*
 * Platform binary types for linking
 */
#ifdef AARCH32
#define PLATFORM_LINKER_FORMAT          "elf32-littlearm"
#define PLATFORM_LINKER_ARCH            arm
#else
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64
#endif /* AARCH32 */

/*
 * Generic platform constants
 */
#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

/*
 * Some of the platform porting definitions use the 'ull' suffix in order to
 * avoid subtle integer overflow errors due to implicit integer type promotion
 * when working with 32-bit values.
 *
 * The TSP linker script includes some of these definitions to define the BL32
 * memory map, but the GNU LD does not support the 'ull' suffix, causing the
 * build process to fail. To solve this problem, the auxiliary macro MAKE_ULL(x)
 * will add the 'ull' suffix only when the macro __LINKER__  is not defined
 * (__LINKER__ is defined in the command line to preprocess the linker script).
 * Constants in the linker script will not have the 'ull' suffix, but this is
 * not a problem since the linker evaluates all constant expressions to 64 bit
 * (assuming the target architecture is 64 bit).
 */
#ifndef __LINKER__
  #define MAKE_ULL(x)			x##ull
#else
  #define MAKE_ULL(x)			x
#endif

#if LOAD_IMAGE_V2
#define BL2_IMAGE_DESC {				\
	.image_id = BL2_IMAGE_ID,			\
	SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,	\
		VERSION_2, image_info_t, 0),		\
	.image_info.image_base = BL2_BASE,		\
	.image_info.image_max_size = BL2_LIMIT - BL2_BASE,\
	SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,	\
		VERSION_2, entry_point_info_t, SECURE | EXECUTABLE),\
	.ep_info.pc = BL2_BASE,				\
}
#else /* LOAD_IMAGE_V2 */
#define BL2_IMAGE_DESC {				\
	.image_id = BL2_IMAGE_ID,			\
	SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,	\
		VERSION_1, image_info_t, 0),		\
	.image_info.image_base = BL2_BASE,		\
	SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,	\
		VERSION_1, entry_point_info_t, SECURE | EXECUTABLE),\
	.ep_info.pc = BL2_BASE,				\
}
#endif /* LOAD_IMAGE_V2 */

/*
 * The following constants identify the extents of the code & read-only data
 * regions. These addresses are used by the MMU setup code and therefore they
 * must be page-aligned.
 *
 * When the code and read-only data are mapped as a single atomic section
 * (i.e. when SEPARATE_CODE_AND_RODATA=0) then we treat the whole section as
 * code by specifying the read-only data section as empty.
 *
 * BL1 is different than the other images in the sense that its read-write data
 * originally lives in Trusted ROM and needs to be relocated in Trusted SRAM at
 * run-time. Therefore, the read-write data in ROM can be mapped with the same
 * memory attributes as the read-only data region. For this reason, BL1 uses
 * different macros.
 *
 * Note that BL1_ROM_END is not necessarily aligned on a page boundary as it
 * just points to the end of BL1's actual content in Trusted ROM. Therefore it
 * needs to be rounded up to the next page size in order to map the whole last
 * page of it with the right memory attributes.
 */
#if SEPARATE_CODE_AND_RODATA
#define BL_CODE_BASE		(unsigned long)(&__TEXT_START__)
#define BL_CODE_LIMIT		(unsigned long)(&__TEXT_END__)
#define BL_RO_DATA_BASE		(unsigned long)(&__RODATA_START__)
#define BL_RO_DATA_LIMIT	(unsigned long)(&__RODATA_END__)

#define BL1_CODE_LIMIT		BL_CODE_LIMIT
#define BL1_RO_DATA_BASE	(unsigned long)(&__RODATA_START__)
#define BL1_RO_DATA_LIMIT	round_up(BL1_ROM_END, PAGE_SIZE)
#else
#define BL_CODE_BASE		(unsigned long)(&__RO_START__)
#define BL_CODE_LIMIT		(unsigned long)(&__RO_END__)
#define BL_RO_DATA_BASE		0
#define BL_RO_DATA_LIMIT	0

#define BL1_CODE_LIMIT		round_up(BL1_ROM_END, PAGE_SIZE)
#define BL1_RO_DATA_BASE	0
#define BL1_RO_DATA_LIMIT	0
#endif /* SEPARATE_CODE_AND_RODATA */

#endif /* __COMMON_DEF_H__ */
