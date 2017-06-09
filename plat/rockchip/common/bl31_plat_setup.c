/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <plat_private.h>
#include <platform_def.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
unsigned long __RO_START__;
unsigned long __RO_END__;

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

#pragma weak params_early_setup
void params_early_setup(void *plat_param_from_bl2)
{
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	console_init(PLAT_RK_UART_BASE, PLAT_RK_UART_CLOCK,
		     PLAT_RK_UART_BAUDRATE);

	VERBOSE("bl31_setup\n");

	/* Passing a NULL context is a critical programming error */
	assert(from_bl2);

	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	bl32_ep_info = *from_bl2->bl32_ep_info;
	bl33_ep_info = *from_bl2->bl33_ep_info;

	/* there may have some board sepcific message need to initialize */
	params_early_setup(plat_params_from_bl2);
}

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	generic_delay_timer_init();
	plat_rockchip_soc_init();

	/* Initialize the gic cpu and distributor interfaces */
	plat_rockchip_gic_driver_init();
	plat_rockchip_gic_init();
	plat_rockchip_pmu_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	plat_cci_init();
	plat_cci_enable();
	plat_configure_mmu_el3(BL31_RO_BASE,
			       BL_COHERENT_RAM_END - BL31_RO_BASE,
			       BL31_RO_BASE,
			       BL31_RO_LIMIT,
			       BL_COHERENT_RAM_BASE,
			       BL_COHERENT_RAM_END);
}
