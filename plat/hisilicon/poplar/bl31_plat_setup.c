/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bl31.h>
#include <bl_common.h>
#include <console.h>
#include <cortex_a53.h>
#include <debug.h>
#include <errno.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform.h>
#include <stddef.h>
#include <string.h>
#include "hi3798cv200.h"
#include "plat_private.h"
#include "platform_def.h"

/* Memory ranges for code and RO data sections */
#define BL31_RO_BASE	(unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT	(unsigned long)(&__RO_END__)

/* Memory ranges for coherent memory section */
#define BL31_COHERENT_RAM_BASE	(unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT	(unsigned long)(&__COHERENT_RAM_END__)

#define TZPC_SEC_ATTR_CTRL_VALUE (0x9DB98D45)

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

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

void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	console_init(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ, PL011_BAUDRATE);

	/* Init console for crash report */
	plat_crash_console_init();


	/*
	 * Copy BL32 (if populated by BL2) and BL33 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	if (from_bl2->bl32_ep_info)
		bl32_image_ep_info = *from_bl2->bl32_ep_info;
	bl33_image_ep_info = *from_bl2->bl33_ep_info;
}

void bl31_platform_setup(void)
{
	/* Init arch timer */
	generic_delay_timer_init();

	/* Init GIC distributor and CPU interface */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

	/* Init security properties of IP blocks */
	hisi_tzpc_sec_init();
}

void bl31_plat_runtime_setup(void)
{
	/* do nothing */
}

void bl31_plat_arch_setup(void)
{
	plat_configure_mmu_el3(BL31_RO_BASE,
			       (BL31_COHERENT_RAM_LIMIT - BL31_RO_BASE),
			       BL31_RO_BASE,
			       BL31_RO_LIMIT,
			       BL31_COHERENT_RAM_BASE,
			       BL31_COHERENT_RAM_LIMIT);

	INFO("Boot BL33 from 0x%lx for %lu Bytes\n",
	     bl33_image_ep_info.pc, bl33_image_ep_info.args.arg2);
}
