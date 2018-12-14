/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/image_decompress.h>

static uintptr_t decompressor_buf_base;
static uint32_t decompressor_buf_size;
static decompressor_t *decompressor;
static struct image_info saved_image_info;

void image_decompress_init(uintptr_t buf_base, uint32_t buf_size,
			   decompressor_t *_decompressor)
{
	decompressor_buf_base = buf_base;
	decompressor_buf_size = buf_size;
	decompressor = _decompressor;
}

void image_decompress_prepare(struct image_info *info)
{
	/*
	 * If the image is compressed, it should be loaded into the temporary
	 * buffer instead of its final destination.  We save image_info, then
	 * override ->image_base and ->image_max_size so that load_image() will
	 * transfer the compressed data to the temporary buffer.
	 */
	saved_image_info = *info;
	info->image_base = decompressor_buf_base;
	info->image_max_size = decompressor_buf_size;
}

int image_decompress(struct image_info *info)
{
	uintptr_t compressed_image_base, image_base, work_base;
	uint32_t compressed_image_size, work_size;
	int ret;

	/*
	 * The size of compressed data has been filled by load_image().
	 * Read it out before restoring image_info.
	 */
	compressed_image_size = info->image_size;
	compressed_image_base = info->image_base;
	*info = saved_image_info;

	assert(compressed_image_size <= decompressor_buf_size);

	image_base = info->image_base;

	/*
	 * Use the rest of the temporary buffer as workspace of the
	 * decompressor since the decompressor may need additional memory.
	 */
	work_base = compressed_image_base + compressed_image_size;
	work_size = decompressor_buf_size - compressed_image_size;

	ret = decompressor(&compressed_image_base, compressed_image_size,
			   &image_base, info->image_max_size,
			   work_base, work_size);
	if (ret) {
		ERROR("Failed to decompress image (err=%d)\n", ret);
		return ret;
	}

	/* image_base is updated to the final pos when decompressor() exits. */
	info->image_size = image_base - info->image_base;

	flush_dcache_range(info->image_base, info->image_size);

	return 0;
}
