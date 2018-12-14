/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OPTEE_UTILS_H
#define OPTEE_UTILS_H

#include <common/bl_common.h>

int parse_optee_header(entry_point_info_t *header_ep,
	image_info_t *pager_image_info,
	image_info_t *paged_image_info);

#endif /* OPTEE_UTILS_H */
