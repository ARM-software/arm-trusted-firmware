/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BL1_PRIVATE_H__
#define __BL1_PRIVATE_H__

#include <types.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will tell us where BL1 lives
 * in Trusted ROM and RAM
 ******************************************************************************/
extern uintptr_t __BL1_ROM_END__;
#define BL1_ROM_END (uintptr_t)(&__BL1_ROM_END__)

extern uintptr_t __BL1_RAM_START__;
extern uintptr_t __BL1_RAM_END__;
#define BL1_RAM_BASE (uintptr_t)(&__BL1_RAM_START__)
#define BL1_RAM_LIMIT (uintptr_t)(&__BL1_RAM_END__)

/******************************************
 * Function prototypes
 *****************************************/
void bl1_arch_setup(void);
void bl1_arch_next_el_setup(void);

void bl1_prepare_next_image(unsigned int image_id);

register_t bl1_fwu_smc_handler(unsigned int smc_fid,
		register_t x1,
		register_t x2,
		register_t x3,
		register_t x4,
		void *cookie,
		void *handle,
		unsigned int flags);
#endif /* __BL1_PRIVATE_H__ */
