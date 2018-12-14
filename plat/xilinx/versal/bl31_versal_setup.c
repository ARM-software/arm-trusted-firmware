/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <plat/common/platform.h>

#include "versal_private.h"

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
static console_pl011_t versal_runtime_console;

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
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{

	/* Initialize the console to provide early debug support */
	int rc = console_pl011_register(VERSAL_UART_BASE,
					VERSAL_UART_CLOCK,
					VERSAL_UART_BAUDRATE,
					&versal_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&versal_runtime_console.console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);

	/* Initialize the platform config for future decision making */
	versal_config_setup();
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(arg0 == 0U);
	assert(arg1 == 0U);

	/*
	 * Do initial security configuration to allow DRAM/device access. On
	 * Base VERSAL only DRAM security is programmable (via TrustZone), but
	 * other platforms might have more programmable security devices
	 * present.
	 */

	/* Populate common information for BL32 and BL33 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* use build time defaults in JTAG boot mode */
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);

	NOTICE("BL31: Secure code at 0x%lx\n", bl32_image_ep_info.pc);
	NOTICE("BL31: Non secure code at 0x%lx\n", bl33_image_ep_info.pc);
}

void bl31_platform_setup(void)
{
	/* Initialize the gic cpu and distributor interfaces */
	plat_versal_gic_driver_init();
	plat_versal_gic_init();
}

void bl31_plat_runtime_setup(void)
{
}

/*
 * Perform the very early platform specific architectural setup here.
 */
void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
				MT_RO_DATA | MT_SECURE),
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
				BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
				MT_DEVICE | MT_RW | MT_SECURE),
		{0}
	};

	setup_page_tables(bl_regions, plat_versal_get_mmap());
	enable_mmu_el3(0);
}
