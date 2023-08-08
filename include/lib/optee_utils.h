/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
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

/*
 * load_addr_hi and load_addr_lo: image load address.
 * image_id: 0 - pager, 1 - paged
 * size: image size in bytes.
 */
typedef struct optee_image {
	uint32_t load_addr_hi;
	uint32_t load_addr_lo;
	uint32_t image_id;
	uint32_t size;
} optee_image_t;

#define OPTEE_PAGER_IMAGE_ID		0
#define OPTEE_PAGED_IMAGE_ID		1

#define OPTEE_MAX_NUM_IMAGES		2u

#define TEE_MAGIC_NUM_OPTEE		0x4554504f
/*
 * magic: header magic number.
 * version: OPTEE header version:
 *		1 - not supported
 *		2 - supported
 * arch: OPTEE os architecture type: 0 - AARCH32, 1 - AARCH64.
 * flags: unused currently.
 * nb_images: number of images.
 */
typedef struct optee_header {
	uint32_t magic;
	uint8_t version;
	uint8_t arch;
	uint16_t flags;
	uint32_t nb_images;
	optee_image_t optee_image_list[];
} optee_header_t;

#endif /* OPTEE_UTILS_H */
