/*
 * Copyright (c) 2019-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/nand.h>
#include <lib/utils.h>

/*
 * Define a single nand_device used by specific NAND frameworks.
 */
static struct nand_device nand_dev;
static uint8_t scratch_buff[PLATFORM_MTD_MAX_PAGE_SIZE];

int nand_read(unsigned int offset, uintptr_t buffer, size_t length,
	      size_t *length_read)
{
	unsigned int block = offset / nand_dev.block_size;
	unsigned int end_block = (offset + length - 1U) / nand_dev.block_size;
	unsigned int page_start =
		(offset % nand_dev.block_size) / nand_dev.page_size;
	unsigned int nb_pages = nand_dev.block_size / nand_dev.page_size;
	unsigned int start_offset = offset % nand_dev.page_size;
	unsigned int page;
	unsigned int bytes_read;
	int is_bad;
	int ret;

	VERBOSE("Block %u - %u, page_start %u, nb %u, length %zu, offset %u\n",
		block, end_block, page_start, nb_pages, length, offset);

	*length_read = 0UL;

	if (((start_offset != 0U) || (length % nand_dev.page_size) != 0U) &&
	    (sizeof(scratch_buff) < nand_dev.page_size)) {
		return -EINVAL;
	}

	while (block <= end_block) {
		is_bad = nand_dev.mtd_block_is_bad(block);
		if (is_bad < 0) {
			return is_bad;
		}

		if (is_bad == 1) {
			/* Skip the block */
			uint32_t max_block =
				nand_dev.size / nand_dev.block_size;

			block++;
			end_block++;
			if ((block < max_block) && (end_block < max_block)) {
				continue;
			}

			return -EIO;
		}

		for (page = page_start; page < nb_pages; page++) {
			if ((start_offset != 0U) ||
			    (length < nand_dev.page_size)) {
				ret = nand_dev.mtd_read_page(
						&nand_dev,
						(block * nb_pages) + page,
						(uintptr_t)scratch_buff);
				if (ret != 0) {
					return ret;
				}

				bytes_read = MIN((size_t)(nand_dev.page_size -
							  start_offset),
						 length);

				memcpy((uint8_t *)buffer,
				       scratch_buff + start_offset,
				       bytes_read);

				start_offset = 0U;
			} else {
				ret = nand_dev.mtd_read_page(&nand_dev,
						(block * nb_pages) + page,
						buffer);
				if (ret != 0) {
					return ret;
				}

				bytes_read = nand_dev.page_size;
			}

			length -= bytes_read;
			buffer += bytes_read;
			*length_read += bytes_read;

			if (length == 0U) {
				break;
			}
		}

		page_start = 0U;
		block++;
	}

	return 0;
}

int nand_seek_bb(uintptr_t base, unsigned int offset, size_t *extra_offset)
{
	unsigned int block;
	unsigned int offset_block;
	unsigned int max_block;
	int is_bad;
	size_t count_bb = 0U;

	block = base / nand_dev.block_size;

	if (offset != 0U) {
		offset_block = (base + offset - 1U) / nand_dev.block_size;
	} else {
		offset_block = block;
	}

	max_block = nand_dev.size / nand_dev.block_size;

	while (block <= offset_block) {
		if (offset_block >= max_block) {
			return -EIO;
		}

		is_bad = nand_dev.mtd_block_is_bad(block);
		if (is_bad < 0) {
			return is_bad;
		}

		if (is_bad == 1) {
			count_bb++;
			offset_block++;
		}

		block++;
	}

	*extra_offset = count_bb * nand_dev.block_size;

	return 0;
}

struct nand_device *get_nand_device(void)
{
	return &nand_dev;
}
