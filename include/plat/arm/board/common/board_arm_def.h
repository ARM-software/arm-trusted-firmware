/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
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
# define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL31)
#if ENABLE_SPM
# define PLATFORM_STACK_SIZE 0x500
#elif PLAT_XLAT_TABLES_DYNAMIC
# define PLATFORM_STACK_SIZE 0x800
#else
# define PLATFORM_STACK_SIZE 0x400
#endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE 0x440
#endif

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

#define PLAT_ARM_TRUSTED_SRAM_SIZE	0x00040000	/* 256 KB */

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FIP_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_FIP_MAX_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/*
 * Map mem_protect flash region with read and write permissions
 */
#define ARM_V2M_MAP_MEM_PROTECT		MAP_REGION_FLAT(PLAT_ARM_MEM_PROT_ADDR,	\
						V2M_FLASH_BLOCK_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#endif /* __BOARD_ARM_DEF_H__ */
