/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef COMMON_DEF_H
#define COMMON_DEF_H

#include <platform_def.h>

#include <common/bl_common.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

/******************************************************************************
 * Required platform porting definitions that are expected to be common to
 * all platforms
 *****************************************************************************/

/*
 * Platform binary types for linking
 */
#ifdef __aarch64__
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64
#else
#define PLATFORM_LINKER_FORMAT          "elf32-littlearm"
#define PLATFORM_LINKER_ARCH            arm
#endif /* __aarch64__ */

/*
 * Generic platform constants
 */
#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

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

#define BL1_CODE_END		BL_CODE_END
#define BL1_RO_DATA_BASE	BL_RO_DATA_BASE
#define BL1_RO_DATA_END		round_up(BL1_ROM_END, PAGE_SIZE)
#if BL2_IN_XIP_MEM
#define BL2_CODE_END		BL_CODE_END
#define BL2_RO_DATA_BASE	BL_RO_DATA_BASE
#define BL2_RO_DATA_END		round_up(BL2_ROM_END, PAGE_SIZE)
#endif /* BL2_IN_XIP_MEM */
#else
#define BL_RO_DATA_BASE		UL(0)
#define BL_RO_DATA_END		UL(0)
#define BL1_CODE_END		round_up(BL1_ROM_END, PAGE_SIZE)
#if BL2_IN_XIP_MEM
#define BL2_RO_DATA_BASE	UL(0)
#define BL2_RO_DATA_END		UL(0)
#define BL2_CODE_END		round_up(BL2_ROM_END, PAGE_SIZE)
#endif /* BL2_IN_XIP_MEM */
#endif /* SEPARATE_CODE_AND_RODATA */

#endif /* COMMON_DEF_H */
