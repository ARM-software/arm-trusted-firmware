/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/auth/auth_mod.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>

#include "bl1_private.h"

/*
 * Function declarations.
 */
static int bl1_fwu_image_copy(unsigned int image_id,
			uintptr_t image_src,
			unsigned int block_size,
			unsigned int image_size,
			unsigned int flags);
static int bl1_fwu_image_auth(unsigned int image_id,
			uintptr_t image_src,
			unsigned int image_size,
			unsigned int flags);
static int bl1_fwu_image_execute(unsigned int image_id,
			void **handle,
			unsigned int flags);
static register_t bl1_fwu_image_resume(register_t image_param,
			void **handle,
			unsigned int flags);
static int bl1_fwu_sec_image_done(void **handle,
			unsigned int flags);
static int bl1_fwu_image_reset(unsigned int image_id,
			unsigned int flags);
__dead2 static void bl1_fwu_done(void *client_cookie, void *reserved);

/*
 * This keeps track of last executed secure image id.
 */
static unsigned int sec_exec_image_id = INVALID_IMAGE_ID;

/*******************************************************************************
 * Top level handler for servicing FWU SMCs.
 ******************************************************************************/
u_register_t bl1_fwu_smc_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			unsigned int flags)
{

	switch (smc_fid) {
	case FWU_SMC_IMAGE_COPY:
		SMC_RET1(handle, bl1_fwu_image_copy((uint32_t)x1, x2,
			(uint32_t)x3, (uint32_t)x4, flags));

	case FWU_SMC_IMAGE_AUTH:
		SMC_RET1(handle, bl1_fwu_image_auth((uint32_t)x1, x2,
			(uint32_t)x3, flags));

	case FWU_SMC_IMAGE_EXECUTE:
		SMC_RET1(handle, bl1_fwu_image_execute((uint32_t)x1, &handle,
			flags));

	case FWU_SMC_IMAGE_RESUME:
		SMC_RET1(handle, bl1_fwu_image_resume((register_t)x1, &handle,
			flags));

	case FWU_SMC_SEC_IMAGE_DONE:
		SMC_RET1(handle, bl1_fwu_sec_image_done(&handle, flags));

	case FWU_SMC_IMAGE_RESET:
		SMC_RET1(handle, bl1_fwu_image_reset((uint32_t)x1, flags));

	case FWU_SMC_UPDATE_DONE:
		bl1_fwu_done((void *)x1, NULL);

	default:
		assert(false); /* Unreachable */
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/*******************************************************************************
 * Utility functions to keep track of the images that are loaded at any time.
 ******************************************************************************/

#ifdef PLAT_FWU_MAX_SIMULTANEOUS_IMAGES
#define FWU_MAX_SIMULTANEOUS_IMAGES	PLAT_FWU_MAX_SIMULTANEOUS_IMAGES
#else
#define FWU_MAX_SIMULTANEOUS_IMAGES	10
#endif

static unsigned int bl1_fwu_loaded_ids[FWU_MAX_SIMULTANEOUS_IMAGES] = {
	[0 ... FWU_MAX_SIMULTANEOUS_IMAGES-1] = INVALID_IMAGE_ID
};

/*
 * Adds an image_id to the bl1_fwu_loaded_ids array.
 * Returns 0 on success, 1 on error.
 */
static int bl1_fwu_add_loaded_id(unsigned int image_id)
{
	int i;

	/* Check if the ID is already in the list */
	for (i = 0; i < FWU_MAX_SIMULTANEOUS_IMAGES; i++) {
		if (bl1_fwu_loaded_ids[i] == image_id)
			return 0;
	}

	/* Find an empty slot */
	for (i = 0; i < FWU_MAX_SIMULTANEOUS_IMAGES; i++) {
		if (bl1_fwu_loaded_ids[i] == INVALID_IMAGE_ID) {
			bl1_fwu_loaded_ids[i] = image_id;
			return 0;
		}
	}

	return 1;
}

/*
 * Removes an image_id from the bl1_fwu_loaded_ids array.
 * Returns 0 on success, 1 on error.
 */
static int bl1_fwu_remove_loaded_id(unsigned int image_id)
{
	int i;

	/* Find the ID */
	for (i = 0; i < FWU_MAX_SIMULTANEOUS_IMAGES; i++) {
		if (bl1_fwu_loaded_ids[i] == image_id) {
			bl1_fwu_loaded_ids[i] = INVALID_IMAGE_ID;
			return 0;
		}
	}

	return 1;
}

/*******************************************************************************
 * This function checks if the specified image overlaps another image already
 * loaded. It returns 0 if there is no overlap, a negative error code otherwise.
 ******************************************************************************/
static int bl1_fwu_image_check_overlaps(unsigned int image_id)
{
	const image_desc_t *desc, *checked_desc;
	const image_info_t *info, *checked_info;

	uintptr_t image_base, image_end;
	uintptr_t checked_image_base, checked_image_end;

	checked_desc = bl1_plat_get_image_desc(image_id);
	checked_info = &checked_desc->image_info;

	/* Image being checked mustn't be empty. */
	assert(checked_info->image_size != 0);

	checked_image_base = checked_info->image_base;
	checked_image_end = checked_image_base + checked_info->image_size - 1;
	/* No need to check for overflows, it's done in bl1_fwu_image_copy(). */

	for (int i = 0; i < FWU_MAX_SIMULTANEOUS_IMAGES; i++) {

		/* Skip INVALID_IMAGE_IDs and don't check image against itself */
		if ((bl1_fwu_loaded_ids[i] == INVALID_IMAGE_ID) ||
				(bl1_fwu_loaded_ids[i] == image_id))
			continue;

		desc = bl1_plat_get_image_desc(bl1_fwu_loaded_ids[i]);

		/* Only check images that are loaded or being loaded. */
		assert ((desc != NULL) && (desc->state != IMAGE_STATE_RESET));

		info = &desc->image_info;

		/* There cannot be overlaps with an empty image. */
		if (info->image_size == 0)
			continue;

		image_base = info->image_base;
		image_end = image_base + info->image_size - 1;
		/*
		 * Overflows cannot happen. It is checked in
		 * bl1_fwu_image_copy() when the image goes from RESET to
		 * COPYING or COPIED.
		 */
		assert (image_end > image_base);

		/* Check if there are overlaps. */
		if (!((image_end < checked_image_base) ||
		    (checked_image_end < image_base))) {
			VERBOSE("Image with ID %d overlaps existing image with ID %d",
				checked_desc->image_id, desc->image_id);
			return -EPERM;
		}
	}

	return 0;
}

/*******************************************************************************
 * This function is responsible for copying secure images in AP Secure RAM.
 ******************************************************************************/
static int bl1_fwu_image_copy(unsigned int image_id,
			uintptr_t image_src,
			unsigned int block_size,
			unsigned int image_size,
			unsigned int flags)
{
	uintptr_t dest_addr;
	unsigned int remaining;
	image_desc_t *desc;

	/* Get the image descriptor. */
	desc = bl1_plat_get_image_desc(image_id);
	if (desc == NULL) {
		WARN("BL1-FWU: Invalid image ID %u\n", image_id);
		return -EPERM;
	}

	/*
	 * The request must originate from a non-secure caller and target a
	 * secure image. Any other scenario is invalid.
	 */
	if (GET_SECURITY_STATE(flags) == SECURE) {
		WARN("BL1-FWU: Copy not allowed from secure world.\n");
		return -EPERM;
	}
	if (GET_SECURITY_STATE(desc->ep_info.h.attr) == NON_SECURE) {
		WARN("BL1-FWU: Copy not allowed for non-secure images.\n");
		return -EPERM;
	}

	/* Check whether the FWU state machine is in the correct state. */
	if ((desc->state != IMAGE_STATE_RESET) &&
	    (desc->state != IMAGE_STATE_COPYING)) {
		WARN("BL1-FWU: Copy not allowed at this point of the FWU"
			" process.\n");
		return -EPERM;
	}

	if ((image_src == 0U) || (block_size == 0U) ||
	    check_uptr_overflow(image_src, block_size - 1)) {
		WARN("BL1-FWU: Copy not allowed due to invalid image source"
			" or block size\n");
		return -ENOMEM;
	}

	if (desc->state == IMAGE_STATE_COPYING) {
		/*
		 * There must have been at least 1 copy operation for this image
		 * previously.
		 */
		assert(desc->copied_size != 0U);
		/*
		 * The image size must have been recorded in the 1st copy
		 * operation.
		 */
		image_size = desc->image_info.image_size;
		assert(image_size != 0);
		assert(desc->copied_size < image_size);

		INFO("BL1-FWU: Continuing image copy in blocks\n");
	} else { /* desc->state == IMAGE_STATE_RESET */
		INFO("BL1-FWU: Initial call to copy an image\n");

		/*
		 * image_size is relevant only for the 1st copy request, it is
		 * then ignored for subsequent calls for this image.
		 */
		if (image_size == 0) {
			WARN("BL1-FWU: Copy not allowed due to invalid image"
				" size\n");
			return -ENOMEM;
		}

		/* Check that the image size to load is within limit */
		if (image_size > desc->image_info.image_max_size) {
			WARN("BL1-FWU: Image size out of bounds\n");
			return -ENOMEM;
		}

		/* Save the given image size. */
		desc->image_info.image_size = image_size;

		/* Make sure the image doesn't overlap other images. */
		if (bl1_fwu_image_check_overlaps(image_id) != 0) {
			desc->image_info.image_size = 0;
			WARN("BL1-FWU: This image overlaps another one\n");
			return -EPERM;
		}

		/*
		 * copied_size must be explicitly initialized here because the
		 * FWU code doesn't necessarily do it when it resets the state
		 * machine.
		 */
		desc->copied_size = 0;
	}

	/*
	 * If the given block size is more than the total image size
	 * then clip the former to the latter.
	 */
	remaining = image_size - desc->copied_size;
	if (block_size > remaining) {
		WARN("BL1-FWU: Block size is too big, clipping it.\n");
		block_size = remaining;
	}

	/* Make sure the source image is mapped in memory. */
	if (bl1_plat_mem_check(image_src, block_size, flags) != 0) {
		WARN("BL1-FWU: Source image is not mapped.\n");
		return -ENOMEM;
	}

	if (bl1_fwu_add_loaded_id(image_id) != 0) {
		WARN("BL1-FWU: Too many images loaded at the same time.\n");
		return -ENOMEM;
	}

	/* Allow the platform to handle pre-image load before copying */
	if (desc->state == IMAGE_STATE_RESET) {
		if (bl1_plat_handle_pre_image_load(image_id) != 0) {
			ERROR("BL1-FWU: Failure in pre-image load of image id %d\n",
					image_id);
			return -EPERM;
		}
	}

	/* Everything looks sane. Go ahead and copy the block of data. */
	dest_addr = desc->image_info.image_base + desc->copied_size;
	(void)memcpy((void *) dest_addr, (const void *) image_src, block_size);
	flush_dcache_range(dest_addr, block_size);

	desc->copied_size += block_size;
	desc->state = (block_size == remaining) ?
		IMAGE_STATE_COPIED : IMAGE_STATE_COPYING;

	INFO("BL1-FWU: Copy operation successful.\n");
	return 0;
}

/*******************************************************************************
 * This function is responsible for authenticating Normal/Secure images.
 ******************************************************************************/
static int bl1_fwu_image_auth(unsigned int image_id,
			uintptr_t image_src,
			unsigned int image_size,
			unsigned int flags)
{
	int result;
	uintptr_t base_addr;
	unsigned int total_size;
	image_desc_t *desc;

	/* Get the image descriptor. */
	desc = bl1_plat_get_image_desc(image_id);
	if (desc ==  NULL)
		return -EPERM;

	if (GET_SECURITY_STATE(flags) == SECURE) {
		if (desc->state != IMAGE_STATE_RESET) {
			WARN("BL1-FWU: Authentication from secure world "
				"while in invalid state\n");
			return -EPERM;
		}
	} else {
		if (GET_SECURITY_STATE(desc->ep_info.h.attr) == SECURE) {
			if (desc->state != IMAGE_STATE_COPIED) {
				WARN("BL1-FWU: Authentication of secure image "
					"from non-secure world while not in copied state\n");
				return -EPERM;
			}
		} else {
			if (desc->state != IMAGE_STATE_RESET) {
				WARN("BL1-FWU: Authentication of non-secure image "
					"from non-secure world while in invalid state\n");
				return -EPERM;
			}
		}
	}

	if (desc->state == IMAGE_STATE_COPIED) {
		/*
		 * Image is in COPIED state.
		 * Use the stored address and size.
		 */
		base_addr = desc->image_info.image_base;
		total_size = desc->image_info.image_size;
	} else {
		if ((image_src == 0U) || (image_size == 0U) ||
		    check_uptr_overflow(image_src, image_size - 1)) {
			WARN("BL1-FWU: Auth not allowed due to invalid"
				" image source/size\n");
			return -ENOMEM;
		}

		/*
		 * Image is in RESET state.
		 * Check the parameters and authenticate the source image in place.
		 */
		if (bl1_plat_mem_check(image_src, image_size,	\
					desc->ep_info.h.attr) != 0) {
			WARN("BL1-FWU: Authentication arguments source/size not mapped\n");
			return -ENOMEM;
		}

		if (bl1_fwu_add_loaded_id(image_id) != 0) {
			WARN("BL1-FWU: Too many images loaded at the same time.\n");
			return -ENOMEM;
		}

		base_addr = image_src;
		total_size = image_size;

		/* Update the image size in the descriptor. */
		desc->image_info.image_size = total_size;
	}

	/*
	 * Authenticate the image.
	 */
	INFO("BL1-FWU: Authenticating image_id:%d\n", image_id);
	result = auth_mod_verify_img(image_id, (void *)base_addr, total_size);
	if (result != 0) {
		WARN("BL1-FWU: Authentication Failed err=%d\n", result);

		/*
		 * Authentication has failed.
		 * Clear the memory if the image was copied.
		 * This is to prevent an attack where this contains
		 * some malicious code that can somehow be executed later.
		 */
		if (desc->state == IMAGE_STATE_COPIED) {
			/* Clear the memory.*/
			zero_normalmem((void *)base_addr, total_size);
			flush_dcache_range(base_addr, total_size);

			/* Indicate that image can be copied again*/
			desc->state = IMAGE_STATE_RESET;
		}

		/*
		 * Even if this fails it's ok because the ID isn't in the array.
		 * The image cannot be in RESET state here, it is checked at the
		 * beginning of the function.
		 */
		(void)bl1_fwu_remove_loaded_id(image_id);
		return -EAUTH;
	}

	/* Indicate that image is in authenticated state. */
	desc->state = IMAGE_STATE_AUTHENTICATED;

	/* Allow the platform to handle post-image load */
	result = bl1_plat_handle_post_image_load(image_id);
	if (result != 0) {
		ERROR("BL1-FWU: Failure %d in post-image load of image id %d\n",
				result, image_id);
		/*
		 * Panic here as the platform handling of post-image load is
		 * not correct.
		 */
		plat_error_handler(result);
	}

	/*
	 * Flush image_info to memory so that other
	 * secure world images can see changes.
	 */
	flush_dcache_range((uintptr_t)&desc->image_info,
		sizeof(image_info_t));

	INFO("BL1-FWU: Authentication was successful\n");

	return 0;
}

/*******************************************************************************
 * This function is responsible for executing Secure images.
 ******************************************************************************/
static int bl1_fwu_image_execute(unsigned int image_id,
			void **handle,
			unsigned int flags)
{
	/* Get the image descriptor. */
	image_desc_t *desc = bl1_plat_get_image_desc(image_id);

	/*
	 * Execution is NOT allowed if:
	 * image_id is invalid OR
	 * Caller is from Secure world OR
	 * Image is Non-Secure OR
	 * Image is Non-Executable OR
	 * Image is NOT in AUTHENTICATED state.
	 */
	if ((desc == NULL) ||
	    (GET_SECURITY_STATE(flags) == SECURE) ||
	    (GET_SECURITY_STATE(desc->ep_info.h.attr) == NON_SECURE) ||
	    (EP_GET_EXE(desc->ep_info.h.attr) == NON_EXECUTABLE) ||
	    (desc->state != IMAGE_STATE_AUTHENTICATED)) {
		WARN("BL1-FWU: Execution not allowed due to invalid state/args\n");
		return -EPERM;
	}

	INFO("BL1-FWU: Executing Secure image\n");

#ifdef __aarch64__
	/* Save NS-EL1 system registers. */
	cm_el1_sysregs_context_save(NON_SECURE);
#endif

	/* Prepare the image for execution. */
	bl1_prepare_next_image(image_id);

	/* Update the secure image id. */
	sec_exec_image_id = image_id;

#ifdef __aarch64__
	*handle = cm_get_context(SECURE);
#else
	*handle = smc_get_ctx(SECURE);
#endif
	return 0;
}

/*******************************************************************************
 * This function is responsible for resuming execution in the other security
 * world
 ******************************************************************************/
static register_t bl1_fwu_image_resume(register_t image_param,
			void **handle,
			unsigned int flags)
{
	image_desc_t *desc;
	unsigned int resume_sec_state;
	unsigned int caller_sec_state = GET_SECURITY_STATE(flags);

	/* Get the image descriptor for last executed secure image id. */
	desc = bl1_plat_get_image_desc(sec_exec_image_id);
	if (caller_sec_state == NON_SECURE) {
		if (desc == NULL) {
			WARN("BL1-FWU: Resume not allowed due to no available"
				"secure image\n");
			return -EPERM;
		}
	} else {
		/* desc must be valid for secure world callers */
		assert(desc != NULL);
	}

	assert(GET_SECURITY_STATE(desc->ep_info.h.attr) == SECURE);
	assert(EP_GET_EXE(desc->ep_info.h.attr) == EXECUTABLE);

	if (caller_sec_state == SECURE) {
		assert(desc->state == IMAGE_STATE_EXECUTED);

		/* Update the flags. */
		desc->state = IMAGE_STATE_INTERRUPTED;
		resume_sec_state = NON_SECURE;
	} else {
		assert(desc->state == IMAGE_STATE_INTERRUPTED);

		/* Update the flags. */
		desc->state = IMAGE_STATE_EXECUTED;
		resume_sec_state = SECURE;
	}

	INFO("BL1-FWU: Resuming %s world context\n",
		(resume_sec_state == SECURE) ? "secure" : "normal");

#ifdef __aarch64__
	/* Save the EL1 system registers of calling world. */
	cm_el1_sysregs_context_save(caller_sec_state);

	/* Restore the EL1 system registers of resuming world. */
	cm_el1_sysregs_context_restore(resume_sec_state);

	/* Update the next context. */
	cm_set_next_eret_context(resume_sec_state);

	*handle = cm_get_context(resume_sec_state);
#else
	/* Update the next context. */
	cm_set_next_context(cm_get_context(resume_sec_state));

	/* Prepare the smc context for the next BL image. */
	smc_set_next_ctx(resume_sec_state);

	*handle = smc_get_ctx(resume_sec_state);
#endif
	return image_param;
}

/*******************************************************************************
 * This function is responsible for resuming normal world context.
 ******************************************************************************/
static int bl1_fwu_sec_image_done(void **handle, unsigned int flags)
{
	image_desc_t *desc;

	/* Make sure caller is from the secure world */
	if (GET_SECURITY_STATE(flags) == NON_SECURE) {
		WARN("BL1-FWU: Image done not allowed from normal world\n");
		return -EPERM;
	}

	/* Get the image descriptor for last executed secure image id */
	desc = bl1_plat_get_image_desc(sec_exec_image_id);

	/* desc must correspond to a valid secure executing image */
	assert(desc != NULL);
	assert(GET_SECURITY_STATE(desc->ep_info.h.attr) == SECURE);
	assert(EP_GET_EXE(desc->ep_info.h.attr) == EXECUTABLE);
	assert(desc->state == IMAGE_STATE_EXECUTED);

#if ENABLE_ASSERTIONS
	int rc = bl1_fwu_remove_loaded_id(sec_exec_image_id);
	assert(rc == 0);
#else
	bl1_fwu_remove_loaded_id(sec_exec_image_id);
#endif

	/* Update the flags. */
	desc->state = IMAGE_STATE_RESET;
	sec_exec_image_id = INVALID_IMAGE_ID;

	INFO("BL1-FWU: Resuming Normal world context\n");
#ifdef __aarch64__
	/*
	 * Secure world is done so no need to save the context.
	 * Just restore the Non-Secure context.
	 */
	cm_el1_sysregs_context_restore(NON_SECURE);

	/* Update the next context. */
	cm_set_next_eret_context(NON_SECURE);

	*handle = cm_get_context(NON_SECURE);
#else
	/* Update the next context. */
	cm_set_next_context(cm_get_context(NON_SECURE));

	/* Prepare the smc context for the next BL image. */
	smc_set_next_ctx(NON_SECURE);

	*handle = smc_get_ctx(NON_SECURE);
#endif
	return 0;
}

/*******************************************************************************
 * This function provides the opportunity for users to perform any
 * platform specific handling after the Firmware update is done.
 ******************************************************************************/
__dead2 static void bl1_fwu_done(void *client_cookie, void *reserved)
{
	NOTICE("BL1-FWU: *******FWU Process Completed*******\n");

	/*
	 * Call platform done function.
	 */
	bl1_plat_fwu_done(client_cookie, reserved);
	assert(false);
}

/*******************************************************************************
 * This function resets an image to IMAGE_STATE_RESET. It fails if the image is
 * being executed.
 ******************************************************************************/
static int bl1_fwu_image_reset(unsigned int image_id, unsigned int flags)
{
	image_desc_t *desc = bl1_plat_get_image_desc(image_id);

	if ((desc == NULL) || (GET_SECURITY_STATE(flags) == SECURE)) {
		WARN("BL1-FWU: Reset not allowed due to invalid args\n");
		return -EPERM;
	}

	switch (desc->state) {

	case IMAGE_STATE_RESET:
		/* Nothing to do. */
		break;

	case IMAGE_STATE_INTERRUPTED:
	case IMAGE_STATE_AUTHENTICATED:
	case IMAGE_STATE_COPIED:
	case IMAGE_STATE_COPYING:

		if (bl1_fwu_remove_loaded_id(image_id) != 0) {
			WARN("BL1-FWU: Image reset couldn't find the image ID\n");
			return -EPERM;
		}

		if (desc->copied_size != 0U) {
			/* Clear the memory if the image is copied */
			assert(GET_SECURITY_STATE(desc->ep_info.h.attr)
				== SECURE);

			zero_normalmem((void *)desc->image_info.image_base,
					desc->copied_size);
			flush_dcache_range(desc->image_info.image_base,
					desc->copied_size);
		}

		/* Reset status variables */
		desc->copied_size = 0;
		desc->image_info.image_size = 0;
		desc->state = IMAGE_STATE_RESET;

		/* Clear authentication state */
		auth_img_flags[image_id] = 0;

		break;

	case IMAGE_STATE_EXECUTED:
	default:
		assert(false); /* Unreachable */
		break;
	}

	return 0;
}
