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
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform.h>
#include <runtime_svc.h>
#include <string.h>

/*******************************************************************************
 * This function pointer is used to initialise the BL32 image. It's initialized
 * by SPD calling bl31_register_bl32_init after setting up all things necessary
 * for SP execution. In cases where both SPD and SP are absent, or when SPD
 * finds it impossible to execute SP, this pointer is left as NULL
 ******************************************************************************/
static int32_t (*bl32_init)(void);

/*******************************************************************************
 * Variable to indicate whether next image to execute after BL31 is BL33
 * (non-secure & default) or BL32 (secure).
 ******************************************************************************/
static uint32_t next_image_type = NON_SECURE;

/*******************************************************************************
 * Simple function to initialise all BL31 helper libraries.
 ******************************************************************************/
void bl31_lib_init(void)
{
	cm_init();
}

/*******************************************************************************
 * BL31 is responsible for setting up the runtime services for the primary cpu
 * before passing control to the bootloader or an Operating System. This
 * function calls runtime_svc_init() which initializes all registered runtime
 * services. The run time services would setup enough context for the core to
 * swtich to the next exception level. When this function returns, the core will
 * switch to the programmed exception level via. an ERET.
 ******************************************************************************/
void bl31_main(void)
{
	NOTICE("BL31: %s\n", version_string);
	NOTICE("BL31: %s\n", build_message);

	/* Perform remaining generic architectural setup from EL3 */
	bl31_arch_setup();

	/* Perform platform setup in BL31 */
	bl31_platform_setup();

	/* Initialise helper libraries */
	bl31_lib_init();

	/* Initialize the runtime services e.g. psci */
	INFO("BL31: Initializing runtime services\n");
	runtime_svc_init();

	/*
	 * All the cold boot actions on the primary cpu are done. We now need to
	 * decide which is the next image (BL32 or BL33) and how to execute it.
	 * If the SPD runtime service is present, it would want to pass control
	 * to BL32 first in S-EL1. In that case, SPD would have registered a
	 * function to intialize bl32 where it takes responsibility of entering
	 * S-EL1 and returning control back to bl31_main. Once this is done we
	 * can prepare entry into BL33 as normal.
	 */

	/*
	 * If SPD had registerd an init hook, invoke it.
	 */
	if (bl32_init) {
		INFO("BL31: Initializing BL32\n");
		(*bl32_init)();
	}
	/*
	 * We are ready to enter the next EL. Prepare entry into the image
	 * corresponding to the desired security state after the next ERET.
	 */
	bl31_prepare_next_image_entry();

	/*
	 * Perform any platform specific runtime setup prior to cold boot exit
	 * from BL31
	 */
	bl31_plat_runtime_setup();
}

/*******************************************************************************
 * Accessor functions to help runtime services decide which image should be
 * executed after BL31. This is BL33 or the non-secure bootloader image by
 * default but the Secure payload dispatcher could override this by requesting
 * an entry into BL32 (Secure payload) first. If it does so then it should use
 * the same API to program an entry into BL33 once BL32 initialisation is
 * complete.
 ******************************************************************************/
void bl31_set_next_image_type(uint32_t security_state)
{
	assert(sec_state_is_valid(security_state));
	next_image_type = security_state;
}

uint32_t bl31_get_next_image_type(void)
{
	return next_image_type;
}

/*******************************************************************************
 * This function programs EL3 registers and performs other setup to enable entry
 * into the next image after BL31 at the next ERET.
 ******************************************************************************/
void bl31_prepare_next_image_entry(void)
{
	entry_point_info_t *next_image_info;
	uint32_t image_type;

	/* Determine which image to execute next */
	image_type = bl31_get_next_image_type();

	/* Program EL3 registers to enable entry into the next EL */
	next_image_info = bl31_plat_get_next_image_ep_info(image_type);
	assert(next_image_info);
	assert(image_type == GET_SECURITY_STATE(next_image_info->h.attr));

	INFO("BL31: Preparing for EL3 exit to %s world\n",
		(image_type == SECURE) ? "secure" : "normal");
	print_entry_point_info(next_image_info);
	cm_init_my_context(next_image_info);
	cm_prepare_el3_exit(image_type);
}

/*******************************************************************************
 * This function initializes the pointer to BL32 init function. This is expected
 * to be called by the SPD after it finishes all its initialization
 ******************************************************************************/
void bl31_register_bl32_init(int32_t (*func)(void))
{
	bl32_init = func;
}
