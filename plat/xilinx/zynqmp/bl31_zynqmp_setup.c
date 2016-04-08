/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <bl31.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform.h>
#include "zynqmp_private.h"

/*
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 */
extern unsigned long __RO_START__;
extern unsigned long __RO_END__;

extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 */
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));

	if (type == NON_SECURE)
		return &bl33_image_ep_info;

	return &bl32_image_ep_info;
}

/*
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 */
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	console_init(ZYNQMP_UART0_BASE, zynqmp_get_uart_clk(),
		     ZYNQMP_UART_BAUDRATE);

	/* Initialize the platform config for future decision making */
	zynqmp_config_setup();

	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

	/*
	 * Do initial security configuration to allow DRAM/device access. On
	 * Base ZYNQMP only DRAM security is programmable (via TrustZone), but
	 * other platforms might have more programmable security devices
	 * present.
	 */

	/* Populate entry point information for BL32 and BL33 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr_for_bl32_entry();

	NOTICE("BL31: Secure code at 0x%lx\n", bl32_image_ep_info.pc);

	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);

	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	NOTICE("BL31: Non secure code at 0x%lx\n", bl33_image_ep_info.pc);
}

void bl31_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
}

void bl31_plat_runtime_setup(void)
{
}

/*
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU in a quick and dirty way.
 */
void bl31_plat_arch_setup(void)
{
	plat_arm_interconnect_init();
	plat_arm_interconnect_enter_coherency();

	arm_configure_mmu_el3(BL31_RO_BASE,
			      BL31_COHERENT_RAM_LIMIT - BL31_RO_BASE,
			      BL31_RO_BASE,
			      BL31_RO_LIMIT,
			      BL31_COHERENT_RAM_BASE,
			      BL31_COHERENT_RAM_LIMIT);
}
