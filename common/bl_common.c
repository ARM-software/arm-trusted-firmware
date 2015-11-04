/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <auth_mod.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <io_storage.h>
#include <platform.h>
#include <string.h>

unsigned long page_align(unsigned long value, unsigned dir)
{
	unsigned long page_size = 1 << FOUR_KB_SHIFT;

	/* Round up the limit to the next page boundary */
	if (value & (page_size - 1)) {
		value &= ~(page_size - 1);
		if (dir == UP)
			value += page_size;
	}

	return value;
}

static inline unsigned int is_page_aligned (unsigned long addr) {
	const unsigned long page_size = 1 << FOUR_KB_SHIFT;

	return (addr & (page_size - 1)) == 0;
}

/******************************************************************************
 * Determine whether the memory region delimited by 'addr' and 'size' is free,
 * given the extents of free memory.
 * Return 1 if it is free, 0 otherwise.
 *****************************************************************************/
static int is_mem_free(uint64_t free_base, size_t free_size,
		       uint64_t addr, size_t size)
{
	return (addr >= free_base) && (addr + size <= free_base + free_size);
}

/******************************************************************************
 * Inside a given memory region, determine whether a sub-region of memory is
 * closer from the top or the bottom of the encompassing region. Return the
 * size of the smallest chunk of free memory surrounding the sub-region in
 * 'small_chunk_size'.
 *****************************************************************************/
static unsigned int choose_mem_pos(uint64_t mem_start, uint64_t mem_end,
				   uint64_t submem_start, uint64_t submem_end,
				   size_t *small_chunk_size)
{
	size_t top_chunk_size, bottom_chunk_size;

	assert(mem_start <= submem_start);
	assert(submem_start <= submem_end);
	assert(submem_end <= mem_end);
	assert(small_chunk_size != NULL);

	top_chunk_size = mem_end - submem_end;
	bottom_chunk_size = submem_start - mem_start;

	if (top_chunk_size < bottom_chunk_size) {
		*small_chunk_size = top_chunk_size;
		return TOP;
	} else {
		*small_chunk_size = bottom_chunk_size;
		return BOTTOM;
	}
}

/******************************************************************************
 * Reserve the memory region delimited by 'addr' and 'size'. The extents of free
 * memory are passed in 'free_base' and 'free_size' and they will be updated to
 * reflect the memory usage.
 * The caller must ensure the memory to reserve is free.
 *****************************************************************************/
void reserve_mem(uint64_t *free_base, size_t *free_size,
		 uint64_t addr, size_t size)
{
	size_t discard_size;
	size_t reserved_size;
	unsigned int pos;

	assert(free_base != NULL);
	assert(free_size != NULL);
	assert(is_mem_free(*free_base, *free_size, addr, size));

	pos = choose_mem_pos(*free_base, *free_base + *free_size,
			     addr, addr + size,
			     &discard_size);

	reserved_size = size + discard_size;
	*free_size -= reserved_size;

	if (pos == BOTTOM)
		*free_base = addr + size;

	VERBOSE("Reserved 0x%lx bytes (discarded 0x%lx bytes %s)\n",
	     reserved_size, discard_size,
	     pos == TOP ? "above" : "below");
}

static void dump_load_info(unsigned long image_load_addr,
			   unsigned long image_size,
			   const meminfo_t *mem_layout)
{
	INFO("Trying to load image at address 0x%lx, size = 0x%lx\n",
		image_load_addr, image_size);
	INFO("Current memory layout:\n");
	INFO("  total region = [0x%lx, 0x%lx]\n", mem_layout->total_base,
			mem_layout->total_base + mem_layout->total_size);
	INFO("  free region = [0x%lx, 0x%lx]\n", mem_layout->free_base,
			mem_layout->free_base + mem_layout->free_size);
}

/* Generic function to return the size of an image */
unsigned long image_size(unsigned int image_id)
{
	uintptr_t dev_handle;
	uintptr_t image_handle;
	uintptr_t image_spec;
	size_t image_size = 0;
	int io_result;

	/* Obtain a reference to the image by querying the platform layer */
	io_result = plat_get_image_source(image_id, &dev_handle, &image_spec);
	if (io_result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, io_result);
		return 0;
	}

	/* Attempt to access the image */
	io_result = io_open(dev_handle, image_spec, &image_handle);
	if (io_result != 0) {
		WARN("Failed to access image id=%u (%i)\n",
			image_id, io_result);
		return 0;
	}

	/* Find the size of the image */
	io_result = io_size(image_handle, &image_size);
	if ((io_result != 0) || (image_size == 0)) {
		WARN("Failed to determine the size of the image id=%u (%i)\n",
			image_id, io_result);
	}
	io_result = io_close(image_handle);
	/* Ignore improbable/unrecoverable error in 'close' */

	/* TODO: Consider maintaining open device connection from this
	 * bootloader stage
	 */
	io_result = io_dev_close(dev_handle);
	/* Ignore improbable/unrecoverable error in 'dev_close' */

	return image_size;
}

/*******************************************************************************
 * Generic function to load an image at a specific address given a name and
 * extents of free memory. It updates the memory layout if the load is
 * successful, as well as the image information and the entry point information.
 * The caller might pass a NULL pointer for the entry point if it is not
 * interested in this information, e.g. because the image just needs to be
 * loaded in memory but won't ever be executed.
 * Returns 0 on success, a negative error code otherwise.
 ******************************************************************************/
int load_image(meminfo_t *mem_layout,
	       unsigned int image_id,
	       uintptr_t image_base,
	       image_info_t *image_data,
	       entry_point_info_t *entry_point_info)
{
	uintptr_t dev_handle;
	uintptr_t image_handle;
	uintptr_t image_spec;
	size_t image_size;
	size_t bytes_read;
	int io_result;

	assert(mem_layout != NULL);
	assert(image_data != NULL);
	assert(image_data->h.version >= VERSION_1);

	/* Obtain a reference to the image by querying the platform layer */
	io_result = plat_get_image_source(image_id, &dev_handle, &image_spec);
	if (io_result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, io_result);
		return io_result;
	}

	/* Attempt to access the image */
	io_result = io_open(dev_handle, image_spec, &image_handle);
	if (io_result != 0) {
		WARN("Failed to access image id=%u (%i)\n",
			image_id, io_result);
		return io_result;
	}

	INFO("Loading image id=%u at address 0x%lx\n", image_id, image_base);

	/* Find the size of the image */
	io_result = io_size(image_handle, &image_size);
	if ((io_result != 0) || (image_size == 0)) {
		WARN("Failed to determine the size of the image id=%u (%i)\n",
			image_id, io_result);
		goto exit;
	}

	/* Check that the memory where the image will be loaded is free */
	if (!is_mem_free(mem_layout->free_base, mem_layout->free_size,
			 image_base, image_size)) {
		WARN("Failed to reserve memory: 0x%lx - 0x%lx\n",
			image_base, image_base + image_size);
		dump_load_info(image_base, image_size, mem_layout);
		io_result = -ENOMEM;
		goto exit;
	}

	/* We have enough space so load the image now */
	/* TODO: Consider whether to try to recover/retry a partially successful read */
	io_result = io_read(image_handle, image_base, image_size, &bytes_read);
	if ((io_result != 0) || (bytes_read < image_size)) {
		WARN("Failed to load image id=%u (%i)\n", image_id, io_result);
		goto exit;
	}

	/*
	 * Update the memory usage info.
	 * This is done after the actual loading so that it is not updated when
	 * the load is unsuccessful.
	 * If the caller does not provide an entry point, bypass the memory
	 * reservation.
	 */
	if (entry_point_info != NULL) {
		reserve_mem(&mem_layout->free_base, &mem_layout->free_size,
				image_base, image_size);
	} else {
		INFO("Skip reserving memory: 0x%lx - 0x%lx\n",
				image_base, image_base + image_size);
	}

	image_data->image_base = image_base;
	image_data->image_size = image_size;

	if (entry_point_info != NULL)
		entry_point_info->pc = image_base;

	/*
	 * File has been successfully loaded.
	 * Flush the image in TZRAM so that the next EL can see it.
	 */
	flush_dcache_range(image_base, image_size);

	INFO("Image id=%u loaded: 0x%lx - 0x%lx\n", image_id, image_base,
	     image_base + image_size);

exit:
	io_close(image_handle);
	/* Ignore improbable/unrecoverable error in 'close' */

	/* TODO: Consider maintaining open device connection from this bootloader stage */
	io_dev_close(dev_handle);
	/* Ignore improbable/unrecoverable error in 'dev_close' */

	return io_result;
}

/*******************************************************************************
 * Generic function to load and authenticate an image. The image is actually
 * loaded by calling the 'load_image()' function. In addition, this function
 * uses recursion to authenticate the parent images up to the root of trust.
 ******************************************************************************/
int load_auth_image(meminfo_t *mem_layout,
		    unsigned int image_id,
		    uintptr_t image_base,
		    image_info_t *image_data,
		    entry_point_info_t *entry_point_info)
{
	int rc;

#if TRUSTED_BOARD_BOOT
	unsigned int parent_id;

	/* Use recursion to authenticate parent images */
	rc = auth_mod_get_parent_id(image_id, &parent_id);
	if (rc == 0) {
		rc = load_auth_image(mem_layout, parent_id, image_base,
				     image_data, NULL);
		if (rc != 0) {
			return rc;
		}
	}
#endif /* TRUSTED_BOARD_BOOT */

	/* Load the image */
	rc = load_image(mem_layout, image_id, image_base, image_data,
			entry_point_info);
	if (rc != 0) {
		return rc;
	}

#if TRUSTED_BOARD_BOOT
	/* Authenticate it */
	rc = auth_mod_verify_img(image_id,
				 (void *)image_data->image_base,
				 image_data->image_size);
	if (rc != 0) {
		memset((void *)image_data->image_base, 0x00,
		       image_data->image_size);
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
		return -EAUTH;
	}

	/* After working with data, invalidate the data cache */
	inv_dcache_range(image_data->image_base,
			(size_t)image_data->image_size);
#endif /* TRUSTED_BOARD_BOOT */

	return 0;
}

/*******************************************************************************
 * Print the content of an entry_point_info_t structure.
 ******************************************************************************/
void print_entry_point_info(const entry_point_info_t *ep_info)
{
	INFO("Entry point address = 0x%llx\n",
		(unsigned long long) ep_info->pc);
	INFO("SPSR = 0x%lx\n", (unsigned long) ep_info->spsr);

#define PRINT_IMAGE_ARG(n)					\
	VERBOSE("Argument #" #n " = 0x%llx\n",			\
		(unsigned long long) ep_info->args.arg##n)

	PRINT_IMAGE_ARG(0);
	PRINT_IMAGE_ARG(1);
	PRINT_IMAGE_ARG(2);
	PRINT_IMAGE_ARG(3);
	PRINT_IMAGE_ARG(4);
	PRINT_IMAGE_ARG(5);
	PRINT_IMAGE_ARG(6);
	PRINT_IMAGE_ARG(7);
#undef PRINT_IMAGE_ARG
}
