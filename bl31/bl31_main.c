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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <stdio.h>


/*******************************************************************************
 * This function pointer is used to initialise the BL32 image. It's initialized
 * by SPD calling bl31_register_bl32_init after setting up all things necessary
 * for SP execution. In cases where both SPD and SP are absent, or when SPD
 * finds it impossible to execute SP, this pointer is left as NULL
 ******************************************************************************/
static int32_t (*bl32_init)(meminfo_t *);

/*******************************************************************************
 * Variable to indicate whether next image to execute after BL31 is BL33
 * (non-secure & default) or BL32 (secure).
 ******************************************************************************/
static uint32_t next_image_type;

/*******************************************************************************
 * Simple function to initialise all BL31 helper libraries.
 ******************************************************************************/
void bl31_lib_init()
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
#if DEBUG
	unsigned long mpidr = read_mpidr();
#endif

	/* Perform remaining generic architectural setup from EL3 */
	bl31_arch_setup();

	/* Perform platform setup in BL1 */
	bl31_platform_setup();

	printf("BL31 %s\n\r", build_message);

	/* Initialise helper libraries */
	bl31_lib_init();

	/* Initialize the runtime services e.g. psci */
	runtime_svc_init();

	/* Clean caches before re-entering normal world */
	dcsw_op_all(DCCSW);

	/*
	 * Use the more complex exception vectors now that context
	 * management is setup. SP_EL3 should point to a 'cpu_context'
	 * structure which has an exception stack allocated.  The PSCI
	 * service should have set the context.
	 */
	assert(cm_get_context(mpidr, NON_SECURE));
	cm_set_next_eret_context(NON_SECURE);
	write_vbar_el3((uint64_t) runtime_exceptions);
	isb();
	next_image_type = NON_SECURE;

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
	 * If SPD had registerd an init hook, invoke it. Pass it the information
	 * about memory extents
	 */
	if (bl32_init)
		(*bl32_init)(bl31_plat_get_bl32_mem_layout());

	/*
	 * We are ready to enter the next EL. Prepare entry into the image
	 * corresponding to the desired security state after the next ERET.
	 */
	bl31_prepare_next_image_entry();
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
	assert(security_state == NON_SECURE || security_state == SECURE);
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
void bl31_prepare_next_image_entry()
{
	el_change_info_t *next_image_info;
	uint32_t scr, image_type;

	/* Determine which image to execute next */
	image_type = bl31_get_next_image_type();

	/*
	 * Setup minimal architectural state of the next highest EL to
	 * allow execution in it immediately upon entering it.
	 */
	bl31_next_el_arch_setup(image_type);

	/* Program EL3 registers to enable entry into the next EL */
	next_image_info = bl31_get_next_image_info(image_type);
	assert(next_image_info);

	scr = read_scr();
	scr &= ~SCR_NS_BIT;
	if (image_type == NON_SECURE)
		scr |= SCR_NS_BIT;

	scr &= ~SCR_RW_BIT;
	if ((next_image_info->spsr & (1 << MODE_RW_SHIFT)) ==
				(MODE_RW_64 << MODE_RW_SHIFT))
		scr |= SCR_RW_BIT;

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

/*******************************************************************************
 * This function initializes the pointer to BL32 init function. This is expected
 * to be called by the SPD after it finishes all its initialization
 ******************************************************************************/
void bl31_register_bl32_init(int32_t (*func)(meminfo_t *))
{
	bl32_init = func;
}
