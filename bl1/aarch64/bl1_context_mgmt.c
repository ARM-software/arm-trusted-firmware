/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <context.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#include "../bl1_private.h"

/*
 * Following array will be used for context management.
 * There are 2 instances, for the Secure and Non-Secure contexts.
 */
static cpu_context_t bl1_cpu_context[2];

/* Following contains the cpu context pointers. */
static void *bl1_cpu_context_ptr[2];


void *cm_get_context(uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));
	return bl1_cpu_context_ptr[security_state];
}

void cm_set_context(void *context, uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));
	bl1_cpu_context_ptr[security_state] = context;
}

/*******************************************************************************
 * This function prepares the context for Secure/Normal world images.
 * Normal world images are transitioned to EL2(if supported) else EL1.
 ******************************************************************************/
void bl1_prepare_next_image(unsigned int image_id)
{
	unsigned int security_state;
	image_desc_t *image_desc;
	entry_point_info_t *next_bl_ep;

#if CTX_INCLUDE_AARCH32_REGS
	/*
	 * Ensure that the build flag to save AArch32 system registers in CPU
	 * context is not set for AArch64-only platforms.
	 */
	if (el_implemented(1) == EL_IMPL_A64ONLY) {
		ERROR("EL1 supports AArch64-only. Please set build flag "
				"CTX_INCLUDE_AARCH32_REGS = 0\n");
		panic();
	}
#endif

	/* Get the image descriptor. */
	image_desc = bl1_plat_get_image_desc(image_id);
	assert(image_desc);

	/* Get the entry point info. */
	next_bl_ep = &image_desc->ep_info;

	/* Get the image security state. */
	security_state = GET_SECURITY_STATE(next_bl_ep->h.attr);

	/* Setup the Secure/Non-Secure context if not done already. */
	if (!cm_get_context(security_state))
		cm_set_context(&bl1_cpu_context[security_state], security_state);

	/* Prepare the SPSR for the next BL image. */
	if (security_state == SECURE) {
		next_bl_ep->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				   DISABLE_ALL_EXCEPTIONS);
	} else {
		/* Use EL2 if supported; else use EL1. */
		if (el_implemented(2) != EL_IMPL_NONE) {
			next_bl_ep->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);
		} else {
			next_bl_ep->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
			   DISABLE_ALL_EXCEPTIONS);
		}
	}

	/* Allow platform to make change */
	bl1_plat_set_ep_info(image_id, next_bl_ep);

	/* Prepare the context for the next BL image. */
	cm_init_my_context(next_bl_ep);
	cm_prepare_el3_exit(security_state);

	/* Indicate that image is in execution state. */
	image_desc->state = IMAGE_STATE_EXECUTED;

	print_entry_point_info(next_bl_ep);
}
