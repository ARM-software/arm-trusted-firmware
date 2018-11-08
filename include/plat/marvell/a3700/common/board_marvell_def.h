/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef BOARD_MARVELL_DEF_H
#define BOARD_MARVELL_DEF_H

/*
 * Required platform porting definitions common to all ARM
 * development platforms
 */

/* Size of cacheable stacks */
#if IMAGE_BL1
#if TRUSTED_BOARD_BOOT
# define PLATFORM_STACK_SIZE 0x1000
#else
# define PLATFORM_STACK_SIZE 0x440
#endif
#elif IMAGE_BL2
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE 0x1000
# else
#  define PLATFORM_STACK_SIZE 0x400
# endif
#elif IMAGE_BL31
# define PLATFORM_STACK_SIZE 0x400
#elif IMAGE_BL32
# define PLATFORM_STACK_SIZE 0x440
#endif

/*
 * PLAT_MARVELL_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#if IMAGE_BLE
#  define PLAT_MARVELL_MMAP_ENTRIES	3
#endif
#if IMAGE_BL1
#  if TRUSTED_BOARD_BOOT
#   define PLAT_MARVELL_MMAP_ENTRIES	7
#  else
#   define PLAT_MARVELL_MMAP_ENTRIES	6
#  endif	/* TRUSTED_BOARD_BOOT */
#endif
#if IMAGE_BL2
#  define PLAT_MARVELL_MMAP_ENTRIES	8
#endif
#if IMAGE_BL31
#define PLAT_MARVELL_MMAP_ENTRIES	5
#endif

/*
 * Platform specific page table and MMU setup constants
 */
#if IMAGE_BL1
#define MAX_XLAT_TABLES			4
#elif IMAGE_BLE
#  define MAX_XLAT_TABLES		4
#elif IMAGE_BL2
#  define MAX_XLAT_TABLES		4
#elif IMAGE_BL31
# define MAX_XLAT_TABLES		4
#elif IMAGE_BL32
#  define MAX_XLAT_TABLES		4
#endif

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

#define PLAT_MARVELL_TRUSTED_SRAM_SIZE	0x80000	/* 512 KB */

#endif /* BOARD_MARVELL_DEF_H */
