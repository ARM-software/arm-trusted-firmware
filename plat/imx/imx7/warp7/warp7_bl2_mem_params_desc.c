/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <desc_image_load.h>
#include <platform.h>
#include <platform_def.h>

static bl_mem_params_node_t bl2_mem_params_descs[] = {
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs);
