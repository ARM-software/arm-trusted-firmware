/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL2_PRIVATE_H
#define BL2_PRIVATE_H

#include <common/bl_common.h>

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

#endif /* BL2_PRIVATE_H */
