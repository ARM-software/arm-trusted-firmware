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

#include <arch_helpers.h>
#include <assert.h>
#include <context.h>
#include <context_mgmt.h>
#include <platform.h>

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

	/* Get the image descriptor. */
	image_desc = bl1_plat_get_image_desc(image_id);
	assert(image_desc);

	/* Get the entry point info. */
	next_bl_ep = &image_desc->ep_info;

	/* Get the image security state. */
	security_state = GET_SEC_STATE(next_bl_ep->h.attr);

	/* Setup the Secure/Non-Secure context if not done already. */
	if (!cm_get_context(security_state))
		cm_set_context(&bl1_cpu_context[security_state], security_state);

	/* Prepare the SPSR for the next BL image. */
	if (security_state == SECURE) {
		next_bl_ep->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				   DISABLE_ALL_EXCEPTIONS);
	} else {
		/* Use EL2 if supported else use EL1. */
		if (read_id_aa64pfr0_el1() &
			(ID_AA64PFR0_ELX_MASK << ID_AA64PFR0_EL2_SHIFT)) {
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
