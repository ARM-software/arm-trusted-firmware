/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMAGE_DECOMPRESS_H
#define IMAGE_DECOMPRESS_H

#include <stddef.h>
#include <stdint.h>

struct image_info;

typedef int (decompressor_t)(uintptr_t *in_buf, size_t in_len,
			     uintptr_t *out_buf, size_t out_len,
			     uintptr_t work_buf, size_t work_len);

void image_decompress_init(uintptr_t buf_base, uint32_t buf_size,
			   decompressor_t *decompressor);
void image_decompress_prepare(struct image_info *info);
int image_decompress(struct image_info *info);

#endif /* IMAGE_DECOMPRESS_H */
