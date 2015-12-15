/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <arch_helpers.h>
#include <auth_mod.h>
#include <bl1.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <platform_def.h>
#include <smcc_helpers.h>
#include <string.h>
#include "bl1_private.h"

/*
 * Function declarations.
 */
static int bl1_fwu_image_copy(unsigned int image_id,
			uintptr_t image_addr,
			unsigned int block_size,
			unsigned int image_size,
			unsigned int flags);
static int bl1_fwu_image_auth(unsigned int image_id,
			uintptr_t image_addr,
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
__dead2 static void bl1_fwu_done(void *client_cookie, void *reserved);

/*
 * This keeps track of last executed secure image id.
 */
static unsigned int sec_exec_image_id = INVALID_IMAGE_ID;

/*******************************************************************************
 * Top level handler for servicing FWU SMCs.
 ******************************************************************************/
register_t bl1_fwu_smc_handler(unsigned int smc_fid,
			register_t x1,
			register_t x2,
			register_t x3,
			register_t x4,
			void *cookie,
			void *handle,
			unsigned int flags)
{

	switch (smc_fid) {
	case FWU_SMC_IMAGE_COPY:
		SMC_RET1(handle, bl1_fwu_image_copy(x1, x2, x3, x4, flags));

	case FWU_SMC_IMAGE_AUTH:
		SMC_RET1(handle, bl1_fwu_image_auth(x1, x2, x3, flags));

	case FWU_SMC_IMAGE_EXECUTE:
		SMC_RET1(handle, bl1_fwu_image_execute(x1, &handle, flags));

	case FWU_SMC_IMAGE_RESUME:
		SMC_RET1(handle, bl1_fwu_image_resume(x1, &handle, flags));

	case FWU_SMC_SEC_IMAGE_DONE:
		SMC_RET1(handle, bl1_fwu_sec_image_done(&handle, flags));

	case FWU_SMC_UPDATE_DONE:
		bl1_fwu_done((void *)x1, NULL);
		/* We should never return from bl1_fwu_done() */

	default:
		assert(0);
		break;
	}

	SMC_RET0(handle);
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
	uintptr_t base_addr;
	meminfo_t *mem_layout;

	/* Get the image descriptor. */
	image_desc_t *image_desc = bl1_plat_get_image_desc(image_id);

	/* Check if we are in correct state. */
	if ((!image_desc) ||
		((image_desc->state != IMAGE_STATE_RESET) &&
		 (image_desc->state != IMAGE_STATE_COPYING))) {
		WARN("BL1-FWU: Copy not allowed due to invalid state\n");
		return -EPERM;
	}

	/* Only Normal world is allowed to copy a Secure image. */
	if ((GET_SEC_STATE(flags) == SECURE) ||
		(GET_SEC_STATE(image_desc->ep_info.h.attr) == NON_SECURE)) {
		WARN("BL1-FWU: Copy not allowed for Non-Secure "
			 "image from Secure-world\n");
		return -EPERM;
	}

	if ((!image_src) || (!block_size)) {
		WARN("BL1-FWU: Copy not allowed due to invalid image source"
			" or block size\n");
		return -ENOMEM;
	}

	/* Get the image base address. */
	base_addr = image_desc->image_info.image_base;

	if (image_desc->state == IMAGE_STATE_COPYING) {
		/*
		 * If last block is more than expected then
		 * clip the block to the required image size.
		 */
		if (image_desc->image_info.copied_size + block_size >
			 image_desc->image_info.image_size) {
			block_size = image_desc->image_info.image_size -
				image_desc->image_info.copied_size;
			WARN("BL1-FWU: Copy argument block_size > remaining image size."
				" Clipping block_size\n");
		}

		/* Make sure the image src/size is mapped. */
		if (bl1_plat_mem_check(image_src, block_size, flags)) {
			WARN("BL1-FWU: Copy arguments source/size not mapped\n");
			return -ENOMEM;
		}

		INFO("BL1-FWU: Continuing image copy in blocks\n");

		/* Copy image for given block size. */
		base_addr += image_desc->image_info.copied_size;
		image_desc->image_info.copied_size += block_size;
		memcpy((void *)base_addr, (const void *)image_src, block_size);
		flush_dcache_range(base_addr, block_size);

		/* Update the state if last block. */
		if (image_desc->image_info.copied_size ==
				image_desc->image_info.image_size) {
			image_desc->state = IMAGE_STATE_COPIED;
			INFO("BL1-FWU: Image copy in blocks completed\n");
		}
	} else {
		/* This means image is in RESET state and ready to be copied. */
		INFO("BL1-FWU: Fresh call to copy an image\n");

		if (!image_size) {
			WARN("BL1-FWU: Copy not allowed due to invalid image size\n");
			return -ENOMEM;
		}

		/*
		 * If block size is more than total size then
		 * assume block size as the total image size.
		 */
		if (block_size > image_size) {
			block_size = image_size;
			WARN("BL1-FWU: Copy argument block_size > image size."
				" Clipping block_size\n");
		}

		/* Make sure the image src/size is mapped. */
		if (bl1_plat_mem_check(image_src, block_size, flags)) {
			WARN("BL1-FWU: Copy arguments source/size not mapped\n");
			return -ENOMEM;
		}

		/* Find out how much free trusted ram remains after BL1 load */
		mem_layout = bl1_plat_sec_mem_layout();
		if ((image_desc->image_info.image_base < mem_layout->free_base) ||
			 (image_desc->image_info.image_base + image_size >
			  mem_layout->free_base + mem_layout->free_size)) {
			WARN("BL1-FWU: Memory not available to copy\n");
			return -ENOMEM;
		}

		/* Update the image size. */
		image_desc->image_info.image_size = image_size;

		/* Copy image for given size. */
		memcpy((void *)base_addr, (const void *)image_src, block_size);
		flush_dcache_range(base_addr, block_size);

		/* Update the state. */
		if (block_size == image_size) {
			image_desc->state = IMAGE_STATE_COPIED;
			INFO("BL1-FWU: Image is copied successfully\n");
		} else {
			image_desc->state = IMAGE_STATE_COPYING;
			INFO("BL1-FWU: Started image copy in blocks\n");
		}

		image_desc->image_info.copied_size = block_size;
	}

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

	/* Get the image descriptor. */
	image_desc_t *image_desc = bl1_plat_get_image_desc(image_id);
	if (!image_desc)
		return -EPERM;

	if (GET_SEC_STATE(flags) == SECURE) {
		if (image_desc->state != IMAGE_STATE_RESET) {
			WARN("BL1-FWU: Authentication from secure world "
				"while in invalid state\n");
			return -EPERM;
		}
	} else {
		if (GET_SEC_STATE(image_desc->ep_info.h.attr) == SECURE) {
			if (image_desc->state != IMAGE_STATE_COPIED) {
				WARN("BL1-FWU: Authentication of secure image "
					"from non-secure world while not in copied state\n");
				return -EPERM;
			}
		} else {
			if (image_desc->state != IMAGE_STATE_RESET) {
				WARN("BL1-FWU: Authentication of non-secure image "
					"from non-secure world while in invalid state\n");
				return -EPERM;
			}
		}
	}

	if (image_desc->state == IMAGE_STATE_COPIED) {
		/*
		 * Image is in COPIED state.
		 * Use the stored address and size.
		 */
		base_addr = image_desc->image_info.image_base;
		total_size = image_desc->image_info.image_size;
	} else {
		if ((!image_src) || (!image_size)) {
			WARN("BL1-FWU: Auth not allowed due to invalid"
				" image source/size\n");
			return -ENOMEM;
		}

		/*
		 * Image is in RESET state.
		 * Check the parameters and authenticate the source image in place.
		 */
		if (bl1_plat_mem_check(image_src, image_size,	\
					image_desc->ep_info.h.attr)) {
			WARN("BL1-FWU: Authentication arguments source/size not mapped\n");
			return -ENOMEM;
		}

		base_addr = image_src;
		total_size = image_size;

		/* Update the image size in the descriptor. */
		image_desc->image_info.image_size = total_size;
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
		if (image_desc->state == IMAGE_STATE_COPIED) {
			/* Clear the memory.*/
			memset((void *)base_addr, 0, total_size);
			flush_dcache_range(base_addr, total_size);

			/* Indicate that image can be copied again*/
			image_desc->state = IMAGE_STATE_RESET;
		}
		return -EAUTH;
	}

	/* Indicate that image is in authenticated state. */
	image_desc->state = IMAGE_STATE_AUTHENTICATED;

	/*
	 * Flush image_info to memory so that other
	 * secure world images can see changes.
	 */
	flush_dcache_range((unsigned long)&image_desc->image_info,
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
	image_desc_t *image_desc = bl1_plat_get_image_desc(image_id);

	/*
	 * Execution is NOT allowed if:
	 * image_id is invalid OR
	 * Caller is from Secure world OR
	 * Image is Non-Secure OR
	 * Image is Non-Executable OR
	 * Image is NOT in AUTHENTICATED state.
	 */
	if ((!image_desc) ||
		(GET_SEC_STATE(flags) == SECURE) ||
		(GET_SEC_STATE(image_desc->ep_info.h.attr) == NON_SECURE) ||
		(GET_EXEC_STATE(image_desc->image_info.h.attr) == NON_EXECUTABLE) ||
		(image_desc->state != IMAGE_STATE_AUTHENTICATED)) {
		WARN("BL1-FWU: Execution not allowed due to invalid state/args\n");
		return -EPERM;
	}

	INFO("BL1-FWU: Executing Secure image\n");

	/* Save NS-EL1 system registers. */
	cm_el1_sysregs_context_save(NON_SECURE);

	/* Prepare the image for execution. */
	bl1_prepare_next_image(image_id);

	/* Update the secure image id. */
	sec_exec_image_id = image_id;

	*handle = cm_get_context(SECURE);
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
	image_desc_t *image_desc;
	unsigned int resume_sec_state;
	unsigned int caller_sec_state = GET_SEC_STATE(flags);

	/* Get the image descriptor for last executed secure image id. */
	image_desc = bl1_plat_get_image_desc(sec_exec_image_id);
	if (caller_sec_state == NON_SECURE) {
		if (!image_desc) {
			WARN("BL1-FWU: Resume not allowed due to no available"
				"secure image\n");
			return -EPERM;
		}
	} else {
		/* image_desc must be valid for secure world callers */
		assert(image_desc);
	}

	assert(GET_SEC_STATE(image_desc->ep_info.h.attr) == SECURE);
	assert(GET_EXEC_STATE(image_desc->image_info.h.attr) == EXECUTABLE);

	if (caller_sec_state == SECURE) {
		assert(image_desc->state == IMAGE_STATE_EXECUTED);

		/* Update the flags. */
		image_desc->state = IMAGE_STATE_INTERRUPTED;
		resume_sec_state = NON_SECURE;
	} else {
		assert(image_desc->state == IMAGE_STATE_INTERRUPTED);

		/* Update the flags. */
		image_desc->state = IMAGE_STATE_EXECUTED;
		resume_sec_state = SECURE;
	}

	/* Save the EL1 system registers of calling world. */
	cm_el1_sysregs_context_save(caller_sec_state);

	/* Restore the EL1 system registers of resuming world. */
	cm_el1_sysregs_context_restore(resume_sec_state);

	/* Update the next context. */
	cm_set_next_eret_context(resume_sec_state);

	INFO("BL1-FWU: Resuming %s world context\n",
		(resume_sec_state == SECURE) ? "secure" : "normal");

	*handle = cm_get_context(resume_sec_state);
	return image_param;
}

/*******************************************************************************
 * This function is responsible for resuming normal world context.
 ******************************************************************************/
static int bl1_fwu_sec_image_done(void **handle, unsigned int flags)
{
	image_desc_t *image_desc;

	/* Make sure caller is from the secure world */
	if (GET_SEC_STATE(flags) == NON_SECURE) {
		WARN("BL1-FWU: Image done not allowed from normal world\n");
		return -EPERM;
	}

	/* Get the image descriptor for last executed secure image id */
	image_desc = bl1_plat_get_image_desc(sec_exec_image_id);

	/* image_desc must correspond to a valid secure executing image */
	assert(image_desc);
	assert(GET_SEC_STATE(image_desc->ep_info.h.attr) == SECURE);
	assert(GET_EXEC_STATE(image_desc->image_info.h.attr) == EXECUTABLE);
	assert(image_desc->state == IMAGE_STATE_EXECUTED);

	/* Update the flags. */
	image_desc->state = IMAGE_STATE_RESET;
	sec_exec_image_id = INVALID_IMAGE_ID;

	/*
	 * Secure world is done so no need to save the context.
	 * Just restore the Non-Secure context.
	 */
	cm_el1_sysregs_context_restore(NON_SECURE);

	/* Update the next context. */
	cm_set_next_eret_context(NON_SECURE);

	INFO("BL1-FWU: Resuming Normal world context\n");

	*handle = cm_get_context(NON_SECURE);
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
	assert(0);
}
