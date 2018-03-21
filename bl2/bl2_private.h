/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BL2_PRIVATE_H__
#define __BL2_PRIVATE_H__

#if BL2_IN_XIP_MEM
/*******************************************************************************
 * Declarations of linker defined symbols which will tell us where BL2 lives
 * in Trusted ROM and RAM
 ******************************************************************************/
extern uintptr_t __BL2_ROM_END__;
#define BL2_ROM_END (uintptr_t)(&__BL2_ROM_END__)

extern uintptr_t __BL2_RAM_START__;
extern uintptr_t __BL2_RAM_END__;
#define BL2_RAM_BASE (uintptr_t)(&__BL2_RAM_START__)
#define BL2_RAM_LIMIT (uintptr_t)(&__BL2_RAM_END__)
#endif

/******************************************
 * Forward declarations
 *****************************************/
struct entry_point_info;

/******************************************
 * Function prototypes
 *****************************************/
void bl2_arch_setup(void);
struct entry_point_info *bl2_load_images(void);
void bl2_run_next_image(const struct entry_point_info *bl_ep_info);

#endif /* __BL2_PRIVATE_H__ */
