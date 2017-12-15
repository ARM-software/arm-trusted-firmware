/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BL2_PRIVATE_H__
#define __BL2_PRIVATE_H__

/******************************************
 * Forward declarations
 *****************************************/
struct entry_point_info;

/******************************************
 * Function prototypes
 *****************************************/
void bl2_arch_setup(void);
struct entry_point_info *bl2_load_images(void);
void bl2_run_next_image(const entry_point_info_t *bl_ep_info);

#endif /* __BL2_PRIVATE_H__ */
