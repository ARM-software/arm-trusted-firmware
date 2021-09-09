/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPTEE_UTILS_H
#define OPTEE_UTILS_H

#include <stdbool.h>

#include <common/bl_common.h>

bool optee_header_is_valid(uintptr_t header_base);

int parse_optee_header(entry_point_info_t *header_ep,
	image_info_t *pager_image_info,
	image_info_t *paged_image_info);

#endif /* OPTEE_UTILS_H */
