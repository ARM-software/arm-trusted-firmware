/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI_SHARED_H
#define RPI_SHARED_H

#include <stddef.h>
#include <stdint.h>

#include <drivers/console.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

/* Serial console functions */
void rpi3_console_init(void);
int rpi3_register_used_uart(console_t *console);

/* Utility functions */
void rpi3_setup_page_tables(uintptr_t total_base, size_t total_size,
			    uintptr_t code_start, uintptr_t code_limit,
			    uintptr_t rodata_start, uintptr_t rodata_limit
#if USE_COHERENT_MEM
			    , uintptr_t coh_start, uintptr_t coh_limit
#endif
			    );

uintptr_t rpi4_get_dtb_address(void);

/* Optional functions required in the Raspberry Pi 3 port */
unsigned int plat_rpi3_calc_core_pos(u_register_t mpidr);

/* BL2 utility functions */
uint32_t rpi3_get_spsr_for_bl32_entry(void);
uint32_t rpi3_get_spsr_for_bl33_entry(void);

/* IO storage utility functions */
void plat_rpi3_io_setup(void);

/* VideoCore firmware commands */
int rpi3_vc_hardware_get_board_revision(uint32_t *revision);

int plat_rpi_get_model(void);

/*******************************************************************************
 * Platform implemented functions
 ******************************************************************************/

void plat_rpi_bl31_custom_setup(void);

#endif /* RPI3_PRIVATE_H */
