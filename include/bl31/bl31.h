/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BL31_H__
#define __BL31_H__

#include <stdint.h>

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
void bl31_next_el_arch_setup(uint32_t security_state);
void bl31_set_next_image_type(uint32_t security_state);
uint32_t bl31_get_next_image_type(void);
void bl31_prepare_next_image_entry(void);
void bl31_register_bl32_init(int32_t (*func)(void));
void bl31_warm_entrypoint(void);
void bl31_main(void);
void bl31_lib_init(void);

#endif /* __BL31_H__ */
