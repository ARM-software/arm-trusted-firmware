/*
 * Copyright (c) 2013-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31_H
#define BL31_H

#include <stdint.h>

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
void bl31_next_el_arch_setup(uint32_t security_state);
void bl31_set_next_image_type(uint32_t security_state);
void bl31_prepare_next_image_entry(void);
void bl31_register_bl32_init(int32_t (*func)(void));
void bl31_register_rmm_init(int32_t (*func)(void));
void bl31_warm_entrypoint(void);
void bl31_main(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		u_register_t arg3);

#endif /* BL31_H */
