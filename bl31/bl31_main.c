/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <semihosting.h>
#include <bl_common.h>
#include <bl31.h>
#include <runtime_svc.h>
#include <context_mgmt.h>

/*******************************************************************************
 * Simple function to initialise all BL31 helper libraries.
 ******************************************************************************/
void bl31_lib_init()
{
	cm_init();
}

/*******************************************************************************
 * BL31 is responsible for setting up the runtime services for the primary cpu
 * before passing control to the bootloader (UEFI) or Linux. This function calls
 * runtime_svc_init() which initializes all registered runtime services. The run
 * time services would setup enough context for the core to swtich to the next
 * exception level. When this function returns, the core will switch to the
 * programmed exception level via. an ERET.
 ******************************************************************************/
void bl31_main(void)
{
	el_change_info *next_image_info;
	uint32_t scr;

	/* Perform remaining generic architectural setup from EL3 */
	bl31_arch_setup();

	/* Perform platform setup in BL1 */
	bl31_platform_setup();

#if defined (__GNUC__)
	printf("BL31 Built : %s, %s\n\r", __TIME__, __DATE__);
#endif
	/* Initialise helper libraries */
	bl31_lib_init();

	/* Initialize the runtime services e.g. psci */
	runtime_svc_init();

	/* Clean caches before re-entering normal world */
	dcsw_op_all(DCCSW);

	/*
	 * Setup minimal architectural state of the next highest EL to
	 * allow execution in it immediately upon entering it.
	 */
	bl31_arch_next_el_setup();

	/* Program EL3 registers to enable entry into the next EL */
	next_image_info = bl31_get_next_image_info();
	scr = read_scr();
	if (next_image_info->security_state == NON_SECURE)
		scr |= SCR_NS_BIT;

	/*
	 * Tell the context mgmt. library to ensure that SP_EL3 points to
	 * the right context to exit from EL3 correctly.
	 */
	cm_set_el3_eret_context(next_image_info->security_state,
			next_image_info->entrypoint,
			next_image_info->spsr,
			scr);

	/* Finally set the next context */
	cm_set_next_eret_context(next_image_info->security_state);
}
