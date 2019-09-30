/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/io/io_storage.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/common/platform.h>

#if TRUSTED_BOARD_BOOT
# ifdef DYN_DISABLE_AUTH
static int disable_auth;

/******************************************************************************
 * API to dynamically disable authentication. Only meant for development
 * systems. This is only invoked if DYN_DISABLE_AUTH is defined.
 *****************************************************************************/
void dyn_disable_auth(void)
{
	INFO("Disabling authentication of images dynamically\n");
	disable_auth = 1;
}
# endif /* DYN_DISABLE_AUTH */

/******************************************************************************
 * Function to determine whether the authentication is disabled dynamically.
 *****************************************************************************/
static int dyn_is_auth_disabled(void)
{
# ifdef DYN_DISABLE_AUTH
	return disable_auth;
# else
	return 0;
# endif
}
#endif /* TRUSTED_BOARD_BOOT */

uintptr_t page_align(uintptr_t value, unsigned dir)
{
	/* Round up the limit to the next page boundary */
	if ((value & (PAGE_SIZE - 1U)) != 0U) {
		value &= ~(PAGE_SIZE - 1U);
		if (dir == UP)
			value += PAGE_SIZE;
	}

	return value;
}

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

	INFO("Loading image id=%u at address 0x%lx\n", image_id, image_base);

	/* Find the size of the image */
	io_result = io_size(image_handle, &image_size);
	if ((io_result != 0) || (image_size == 0U)) {
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

	/*
	 * image_data->image_max_size is a uint32_t so image_size will always
	 * fit in image_data->image_size.
	 */
	image_data->image_size = (uint32_t)image_size;

	/* We have enough space so load the image now */
	/* TODO: Consider whether to try to recover/retry a partially successful read */
	io_result = io_read(image_handle, image_base, image_size, &bytes_read);
	if ((io_result != 0) || (bytes_read < image_size)) {
		WARN("Failed to load image id=%u (%i)\n", image_id, io_result);
		goto exit;
	}

	INFO("Image id=%u loaded: 0x%lx - 0x%lx\n", image_id, image_base,
	     (uintptr_t)(image_base + image_size));

exit:
	(void)io_close(image_handle);
	/* Ignore improbable/unrecoverable error in 'close' */

	/* TODO: Consider maintaining open device connection from this bootloader stage */
	(void)io_dev_close(dev_handle);
	/* Ignore improbable/unrecoverable error in 'dev_close' */

	return io_result;
}

/*
 * Load an image and flush it out to main memory so that it can be executed
 * later by any CPU, regardless of cache and MMU state.
 */
static int load_image_flush(unsigned int image_id,
			    image_info_t *image_data)
{
	int rc;

	rc = load_image(image_id, image_data);
	if (rc == 0) {
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
	}

	return rc;
}


#if TRUSTED_BOARD_BOOT
/*
 * This function uses recursion to authenticate the parent images up to the root
 * of trust.
 */
static int load_auth_image_recursive(unsigned int image_id,
				    image_info_t *image_data,
				    int is_parent_image)
{
	int rc;
	unsigned int parent_id;

	/* Use recursion to authenticate parent images */
	rc = auth_mod_get_parent_id(image_id, &parent_id);
	if (rc == 0) {
		rc = load_auth_image_recursive(parent_id, image_data, 1);
		if (rc != 0) {
			return rc;
		}
	}

	/* Load the image */
	rc = load_image(image_id, image_data);
	if (rc != 0) {
		return rc;
	}

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
	 * Flush the image to main memory so that it can be executed later by
	 * any CPU, regardless of cache and MMU state. This is only needed for
	 * child images, not for the parents (certificates).
	 */
	if (is_parent_image == 0) {
		flush_dcache_range(image_data->image_base,
				   image_data->image_size);
	}

	return 0;
}
#endif /* TRUSTED_BOARD_BOOT */

static int load_auth_image_internal(unsigned int image_id,
				    image_info_t *image_data)
{
#if TRUSTED_BOARD_BOOT
	if (dyn_is_auth_disabled() == 0) {
		return load_auth_image_recursive(image_id, image_data, 0);
	}
#endif

	return load_image_flush(image_id, image_data);
}

/*******************************************************************************
 * Generic function to load and authenticate an image. The image is actually
 * loaded by calling the 'load_image()' function. Therefore, it returns the
 * same error codes if the loading operation failed, or -EAUTH if the
 * authentication failed. In addition, this function uses recursion to
 * authenticate the parent images up to the root of trust (if TBB is enabled).
 ******************************************************************************/
int load_auth_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	do {
		err = load_auth_image_internal(image_id, image_data);
	} while ((err != 0) && (plat_try_next_boot_source() != 0));

	return err;
}

/*******************************************************************************
 * Print the content of an entry_point_info_t structure.
 ******************************************************************************/
void print_entry_point_info(const entry_point_info_t *ep_info)
{
	INFO("Entry point address = 0x%lx\n", ep_info->pc);
	INFO("SPSR = 0x%x\n", ep_info->spsr);

#define PRINT_IMAGE_ARG(n)					\
	VERBOSE("Argument #" #n " = 0x%llx\n",			\
		(unsigned long long) ep_info->args.arg##n)

	PRINT_IMAGE_ARG(0);
	PRINT_IMAGE_ARG(1);
	PRINT_IMAGE_ARG(2);
	PRINT_IMAGE_ARG(3);
#ifdef __aarch64__
	PRINT_IMAGE_ARG(4);
	PRINT_IMAGE_ARG(5);
	PRINT_IMAGE_ARG(6);
	PRINT_IMAGE_ARG(7);
#endif
#undef PRINT_IMAGE_ARG
}
