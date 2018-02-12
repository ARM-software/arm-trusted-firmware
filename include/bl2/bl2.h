/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL2_H__
#define BL2_H__

struct entry_point_info;

void bl2_main(void);
struct entry_point_info *bl2_load_images(void);

#endif /* BL2_H__ */
