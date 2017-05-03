/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
#define BL_CODE_END		(unsigned long)(&__TEXT_END__)
#define BL_RO_DATA_BASE		(unsigned long)(&__RODATA_START__)
#define BL_RO_DATA_END		(unsigned long)(&__RODATA_END__)

#define BL1_CODE_END		BL_CODE_END
#define BL1_RO_DATA_BASE	(unsigned long)(&__RODATA_START__)
#define BL1_RO_DATA_END		round_up(BL1_ROM_END, PAGE_SIZE)
#else
#define BL_CODE_BASE		(unsigned long)(&__RO_START__)
#define BL_CODE_END		(unsigned long)(&__RO_END__)
#define BL_RO_DATA_BASE		0
#define BL_RO_DATA_END		0

#define BL1_CODE_END		round_up(BL1_ROM_END, PAGE_SIZE)
#define BL1_RO_DATA_BASE	0
#define BL1_RO_DATA_END		0
#endif /* SEPARATE_CODE_AND_RODATA */

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL_COHERENT_RAM_BASE	(unsigned long)(&__COHERENT_RAM_START__)
#define BL_COHERENT_RAM_END	(unsigned long)(&__COHERENT_RAM_END__)

#endif /* __COMMON_DEF_H__ */
