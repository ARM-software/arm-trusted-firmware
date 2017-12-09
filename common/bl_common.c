/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
#include <utils.h>
#include <xlat_tables_defs.h>

uintptr_t page_align(uintptr_t value, unsigned dir)
{
	/* Round up the limit to the next page boundary */
	if (value & (PAGE_SIZE - 1)) {
		value &= ~(PAGE_SIZE - 1);
		if (dir == UP)
			value += PAGE_SIZE;
	}

	return value;
}

/******************************************************************************
 * Determine whether the memory region delimited by 'addr' and 'size' is free,
 * given the extents of free memory.
 * Return 1 if it is free, 0 if it is not free or if the input values are
 * invalid.
 *****************************************************************************/
int is_mem_free(uintptr_t free_base, size_t free_size,
		uintptr_t addr, size_t size)
{
	uintptr_t free_end, requested_end;

	/*
	 * Handle corner cases first.
	 *
	 * The order of the 2 tests is important, because if there's no space
	 * left (i.e. free_size == 0) but we don't ask for any memory
	 * (i.e. size == 0) then we should report that the memory is free.
	 */
	if (size == 0)
		return 1;	/* A zero-byte region is always free */
	if (free_size == 0)
		return 0;

	/*
	 * Check that the end addresses don't overflow.
	 * If they do, consider that this memory region is not free, as this
	 * is an invalid scenario.
	 */
	if (check_uptr_overflow(free_base, free_size - 1))
		return 0;
	free_end = free_base + (free_size - 1);

	if (check_uptr_overflow(addr, size - 1))
		return 0;
	requested_end = addr + (size - 1);

	/*
	 * Finally, check that the requested memory region lies within the free
	 * region.
	 */
	return (addr >= free_base) && (requested_end <= free_end);
}

#if !LOAD_IMAGE_V2
/******************************************************************************
 * Inside a given memory region, determine whether a sub-region of memory is
 * closer from the top or the bottom of the encompassing region. Return the
 * size of the smallest chunk of free memory surrounding the sub-region in
 * 'small_chunk_size'.
 *****************************************************************************/
static unsigned int choose_mem_pos(uintptr_t mem_start, uintptr_t mem_end,
				  uintptr_t submem_start, uintptr_t submem_end,
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
 * The caller must ensure the memory to reserve is free and that the addresses
 * and sizes passed in arguments are sane.
 *****************************************************************************/
void reserve_mem(uintptr_t *free_base, size_t *free_size,
		 uintptr_t addr, size_t size)
{
	size_t discard_size;
	size_t reserved_size;
	unsigned int pos;

	assert(free_base != NULL);
	assert(free_size != NULL);
	assert(is_mem_free(*free_base, *free_size, addr, size));

	if (size == 0) {
		WARN("Nothing to allocate, requested size is zero\n");
		return;
	}

	pos = choose_mem_pos(*free_base, *free_base + (*free_size - 1),
			     addr, addr + (size - 1),
			     &discard_size);

	reserved_size = size + discard_size;
	*free_size -= reserved_size;

	if (pos == BOTTOM)
		*free_base = addr + size;

	VERBOSE("Reserved 0x%zx bytes (discarded 0x%zx bytes %s)\n",
	     reserved_size, discard_size,
	     pos == TOP ? "above" : "below");
}

static void dump_load_info(uintptr_t image_load_addr,
			   size_t image_size,
			   const meminfo_t *mem_layout)
{
	INFO("Trying to load image at address %p, size = 0x%zx\n",
		(void *)image_load_addr, image_size);
	INFO("Current memory layout:\n");
	INFO("  total region = [base = %p, size = 0x%zx]\n",
		(void *) mem_layout->total_base, mem_layout->total_size);
	INFO("  free region = [base = %p, size = 0x%zx]\n",
		(void *) mem_layout->free_base, mem_layout->free_size);
}
#endif /* LOAD_IMAGE_V2 */

/* Generic function to return the size of an image */
size_t image_size(unsigned int image_id)
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

#if LOAD_IMAGE_V2

/*******************************************************************************
 * Internal function to load an image at a specific address given
 * an image ID and extents of free memory.
 *
 * If the load is successful then the image information is updated.
 *
 * Returns 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_image(unsigned int image_id, image_info_t *image_data)
{
	uintptr_t dev_handle;
	uintptr_t image_handle;
	uintptr_t image_spec;
	uintptr_t image_base;
	size_t image_size;
	size_t bytes_read;
	int io_result;

	assert(image_data != NULL);
	assert(image_data->h.version >= VERSION_2);

	image_base = image_data->image_base;

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

	INFO("Loading image id=%u at address %p\n", image_id,
		(void *) image_base);

	/* Find the size of the image */
	io_result = io_size(image_handle, &image_size);
	if ((io_result != 0) || (image_size == 0)) {
		WARN("Failed to determine the size of the image id=%u (%i)\n",
			image_id, io_result);
		goto exit;
	}

	/* Check that the image size to load is within limit */
	if (image_size > image_data->image_max_size) {
		WARN("Image id=%u size out of bounds\n", image_id);
		io_result = -EFBIG;
		goto exit;
	}

	image_data->image_size = image_size;

	/* We have enough space so load the image now */
	/* TODO: Consider whether to try to recover/retry a partially successful read */
	io_result = io_read(image_handle, image_base, image_size, &bytes_read);
	if ((io_result != 0) || (bytes_read < image_size)) {
		WARN("Failed to load image id=%u (%i)\n", image_id, io_result);
		goto exit;
	}

	INFO("Image id=%u loaded: %p - %p\n", image_id, (void *) image_base,
	     (void *) (image_base + image_size));

exit:
	io_close(image_handle);
	/* Ignore improbable/unrecoverable error in 'close' */

	/* TODO: Consider maintaining open device connection from this bootloader stage */
	io_dev_close(dev_handle);
	/* Ignore improbable/unrecoverable error in 'dev_close' */

	return io_result;
}

static int load_auth_image_internal(unsigned int image_id,
				    image_info_t *image_data,
				    int is_parent_image)
{
	int rc;

#if TRUSTED_BOARD_BOOT
	unsigned int parent_id;

	/* Use recursion to authenticate parent images */
	rc = auth_mod_get_parent_id(image_id, &parent_id);
	if (rc == 0) {
		rc = load_auth_image_internal(parent_id, image_data, 1);
		if (rc != 0) {
			return rc;
		}
	}
#endif /* TRUSTED_BOARD_BOOT */

	/* Load the image */
	rc = load_image(image_id, image_data);
	if (rc != 0) {
		return rc;
	}

#if TRUSTED_BOARD_BOOT
	/* Authenticate it */
	rc = auth_mod_verify_img(image_id,
				 (void *)image_data->image_base,
				 image_data->image_size);
	if (rc != 0) {
		/* Authentication error, zero memory and flush it right away. */
		zero_normalmem((void *)image_data->image_base,
		       image_data->image_size);
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
		return -EAUTH;
	}
#endif /* TRUSTED_BOARD_BOOT */

	/*
	 * Flush the image to main memory so that it can be executed later by
	 * any CPU, regardless of cache and MMU state. If TBB is enabled, then
	 * the file has been successfully loaded and authenticated and flush
	 * only for child images, not for the parents (certificates).
	 */
	if (!is_parent_image) {
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
	}


	return 0;
}

/*******************************************************************************
 * Generic function to load and authenticate an image. The image is actually
 * loaded by calling the 'load_image()' function. Therefore, it returns the
 * same error codes if the loading operation failed, or -EAUTH if the
 * authentication failed. In addition, this function uses recursion to
 * authenticate the parent images up to the root of trust.
 ******************************************************************************/
int load_auth_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	do {
		err = load_auth_image_internal(image_id, image_data, 0);
	} while (err != 0 && plat_try_next_boot_source());

	return err;
}

#else /* LOAD_IMAGE_V2 */

/*******************************************************************************
 * Generic function to load an image at a specific address given an image ID and
 * extents of free memory.
 *
 * If the load is successful then the image information is updated.
 *
 * If the entry_point_info argument is not NULL then this function also updates:
 * - the memory layout to mark the memory as reserved;
 * - the entry point information.
 *
 * The caller might pass a NULL pointer for the entry point if they are not
 * interested in this information. This is typically the case for non-executable
 * images (e.g. certificates) and executable images that won't ever be executed
 * on the application processor (e.g. additional microcontroller firmware).
 *
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
	assert(image_data->h.version == VERSION_1);

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

	INFO("Loading image id=%u at address %p\n", image_id,
		(void *) image_base);

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
		WARN("Failed to reserve region [base = %p, size = 0x%zx]\n",
		     (void *) image_base, image_size);
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

	image_data->image_base = image_base;
	image_data->image_size = image_size;

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
		entry_point_info->pc = image_base;
	} else {
		INFO("Skip reserving region [base = %p, size = 0x%zx]\n",
		     (void *) image_base, image_size);
	}

#if !TRUSTED_BOARD_BOOT
	/*
	 * File has been successfully loaded.
	 * Flush the image to main memory so that it can be executed later by
	 * any CPU, regardless of cache and MMU state.
	 * When TBB is enabled the image is flushed later, after image
	 * authentication.
	 */
	flush_dcache_range(image_base, image_size);
#endif /* TRUSTED_BOARD_BOOT */

	INFO("Image id=%u loaded at address %p, size = 0x%zx\n", image_id,
		(void *) image_base, image_size);

exit:
	io_close(image_handle);
	/* Ignore improbable/unrecoverable error in 'close' */

	/* TODO: Consider maintaining open device connection from this bootloader stage */
	io_dev_close(dev_handle);
	/* Ignore improbable/unrecoverable error in 'dev_close' */

	return io_result;
}

static int load_auth_image_internal(meminfo_t *mem_layout,
				    unsigned int image_id,
				    uintptr_t image_base,
				    image_info_t *image_data,
				    entry_point_info_t *entry_point_info,
				    int is_parent_image)
{
	int rc;

#if TRUSTED_BOARD_BOOT
	unsigned int parent_id;

	/* Use recursion to authenticate parent images */
	rc = auth_mod_get_parent_id(image_id, &parent_id);
	if (rc == 0) {
		rc = load_auth_image_internal(mem_layout, parent_id, image_base,
				     image_data, NULL, 1);
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
		/* Authentication error, zero memory and flush it right away. */
		zero_normalmem((void *)image_data->image_base,
		       image_data->image_size);
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
		return -EAUTH;
	}
	/*
	 * File has been successfully loaded and authenticated.
	 * Flush the image to main memory so that it can be executed later by
	 * any CPU, regardless of cache and MMU state.
	 * Do it only for child images, not for the parents (certificates).
	 */
	if (!is_parent_image) {
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
	}
#endif /* TRUSTED_BOARD_BOOT */

	return 0;
}

/*******************************************************************************
 * Generic function to load and authenticate an image. The image is actually
 * loaded by calling the 'load_image()' function. Therefore, it returns the
 * same error codes if the loading operation failed, or -EAUTH if the
 * authentication failed. In addition, this function uses recursion to
 * authenticate the parent images up to the root of trust.
 ******************************************************************************/
int load_auth_image(meminfo_t *mem_layout,
		    unsigned int image_id,
		    uintptr_t image_base,
		    image_info_t *image_data,
		    entry_point_info_t *entry_point_info)
{
	int err;

	do {
		err = load_auth_image_internal(mem_layout, image_id, image_base,
					       image_data, entry_point_info, 0);
	} while (err != 0 && plat_try_next_boot_source());

	return err;
}

#endif /* LOAD_IMAGE_V2 */

/*******************************************************************************
 * Print the content of an entry_point_info_t structure.
 ******************************************************************************/
void print_entry_point_info(const entry_point_info_t *ep_info)
{
	INFO("Entry point address = %p\n", (void *)ep_info->pc);
	INFO("SPSR = 0x%x\n", ep_info->spsr);

#define PRINT_IMAGE_ARG(n)					\
	VERBOSE("Argument #" #n " = 0x%llx\n",			\
		(unsigned long long) ep_info->args.arg##n)

	PRINT_IMAGE_ARG(0);
	PRINT_IMAGE_ARG(1);
	PRINT_IMAGE_ARG(2);
	PRINT_IMAGE_ARG(3);
#ifndef AARCH32
	PRINT_IMAGE_ARG(4);
	PRINT_IMAGE_ARG(5);
	PRINT_IMAGE_ARG(6);
	PRINT_IMAGE_ARG(7);
#endif
#undef PRINT_IMAGE_ARG
}
