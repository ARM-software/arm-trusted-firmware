/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RPI3_PRIVATE_H__
#define __RPI3_PRIVATE_H__

#include <sys/types.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

/* Utility functions */
void rpi3_setup_page_tables(uintptr_t total_base, size_t total_size,
			    uintptr_t code_start, uintptr_t code_limit,
			    uintptr_t rodata_start, uintptr_t rodata_limit
#if USE_COHERENT_MEM
			    , uintptr_t coh_start, uintptr_t coh_limit
#endif
			    );

/* Optional functions required in the Raspberry Pi 3 port */
unsigned int plat_rpi3_calc_core_pos(u_register_t mpidr);

/* BL2 utility functions */
uint32_t rpi3_get_spsr_for_bl32_entry(void);
uint32_t rpi3_get_spsr_for_bl33_entry(void);

/* IO storage utility functions */
void plat_rpi3_io_setup(void);

#endif /*__RPI3_PRIVATE_H__ */
