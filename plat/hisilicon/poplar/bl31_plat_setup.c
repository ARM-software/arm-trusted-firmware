/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a53.h>
#include <drivers/arm/pl011.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "hi3798cv200.h"
#include "plat_private.h"

#define TZPC_SEC_ATTR_CTRL_VALUE (0x9DB98D45)

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
static console_pl011_t console;

static void hisi_tzpc_sec_init(void)
{
	mmio_write_32(HISI_TZPC_SEC_ATTR_CTRL, TZPC_SEC_ATTR_CTRL_VALUE);
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;
	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	void *from_bl2;

	from_bl2 = (void *) arg0;

	console_pl011_register(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);

	/* Init console for crash report */
	plat_crash_console_init();

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;

	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0)
		panic();
}

void bl31_platform_setup(void)
{
	/* Init arch timer */
	generic_delay_timer_init();

	/* Init GIC distributor and CPU interface */
	poplar_gic_driver_init();
	poplar_gic_init();

	/* Init security properties of IP blocks */
	hisi_tzpc_sec_init();
}

void bl31_plat_runtime_setup(void)
{
	/* do nothing */
}

void bl31_plat_arch_setup(void)
{
	plat_configure_mmu_el3(BL31_BASE,
			       (BL31_LIMIT - BL31_BASE),
			       BL_CODE_BASE,
			       BL_CODE_END,
			       BL_COHERENT_RAM_BASE,
			       BL_COHERENT_RAM_END);

	INFO("Boot BL33 from 0x%lx for %lu Bytes\n",
	     bl33_image_ep_info.pc, bl33_image_ep_info.args.arg2);
}
