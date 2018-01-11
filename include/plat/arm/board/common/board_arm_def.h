/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __BOARD_ARM_DEF_H__
#define __BOARD_ARM_DEF_H__

#include <v2m_def.h>


/*
 * Required platform porting definitions common to all ARM
 * development platforms
 */

/* Size of cacheable stacks */
#if defined(IMAGE_BL1)
#if TRUSTED_BOARD_BOOT
# define PLATFORM_STACK_SIZE 0x1000
#else
# define PLATFORM_STACK_SIZE 0x440
#endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE 0x1000
# else
#  define PLATFORM_STACK_SIZE 0x400
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE 0x200
#elif defined(IMAGE_BL31)
# define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE 0x440
#endif

/*
 * The constants below are not optimised for memory usage. Platforms that wish
 * to optimise these constants should set `ARM_BOARD_OPTIMISE_MEM` to 1 and
 * provide there own values.
 */
#if !ARM_BOARD_OPTIMISE_MEM
/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 *
 * Provide relatively optimised values for the runtime images (BL31 and BL32).
 * Optimisation is less important for the other, transient boot images so a
 * common, maximum value is used across these images.
 *
 * They are also used for the dynamically mapped regions in the images that
 * enable dynamic memory mapping.
 */
#if defined(IMAGE_BL31)
# if ENABLE_SPM
#  define PLAT_ARM_MMAP_ENTRIES		9
#  define MAX_XLAT_TABLES		7
#  define PLAT_SP_IMAGE_MMAP_REGIONS	7
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	10
# else
#  define PLAT_ARM_MMAP_ENTRIES		7
#  define MAX_XLAT_TABLES		5
# endif
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		7
# define MAX_XLAT_TABLES		5
#else
# define PLAT_ARM_MMAP_ENTRIES		11
# define MAX_XLAT_TABLES		5
#endif

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE	0xB000

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		0x1E000
#else
# define PLAT_ARM_MAX_BL2_SIZE		0xF000
#endif

/*
 * PLAT_ARM_MAX_BL31_SIZE is calculated using the current BL31 debug size plus a
 * little space for growth.
 */
#define PLAT_ARM_MAX_BL31_SIZE		0x20000

#ifdef AARCH32
/*
 * PLAT_ARM_MAX_BL32_SIZE is calculated for SP_MIN as the AArch32 Secure
 * Payload.
 */
# define PLAT_ARM_MAX_BL32_SIZE		0x1D000
#endif

#endif /* ARM_BOARD_OPTIMISE_MEM */

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00040000	/* 256 KB */

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FIP_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_FIP_MAX_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/* PSCI memory protect definitions:
 * This variable is stored in a non-secure flash because some ARM reference
 * platforms do not have secure NVRAM. Real systems that provided MEM_PROTECT
 * support must use a secure NVRAM to store the PSCI MEM_PROTECT definitions.
 */
#define PLAT_ARM_MEM_PROT_ADDR		(V2M_FLASH0_BASE + \
					 V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/*
 * Map mem_protect flash region with read and write permissions
 */
#define ARM_V2M_MAP_MEM_PROTECT		MAP_REGION_FLAT(PLAT_ARM_MEM_PROT_ADDR,	\
						V2M_FLASH_BLOCK_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#endif /* __BOARD_ARM_DEF_H__ */
