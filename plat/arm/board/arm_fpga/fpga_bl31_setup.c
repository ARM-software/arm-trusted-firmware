/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <lib/mmio.h>
#include <drivers/generic_delay_timer.h>

#include <plat/common/platform.h>
#include <platform_def.h>

#include "fpga_private.h"

static entry_point_info_t bl33_image_ep_info;

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return 0;
#endif
}

uint32_t fpga_get_spsr_for_bl33_entry(void)
{
	return SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	fpga_console_init();

	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = fpga_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	/* Set x0-x3 for the primary CPU as expected by the kernel */
	bl33_image_ep_info.args.arg0 = (u_register_t)FPGA_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
}

void bl31_plat_arch_setup(void)
{
}

void bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_fpga_gic_init();

	/* Write frequency to CNTCRL and initialize timer */
	generic_delay_timer_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;
	next_image_info = &bl33_image_ep_info;

	/* Only expecting BL33: the kernel will run in EL2NS */
	assert(type == NON_SECURE);

	/* None of the images can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}

unsigned int plat_get_syscnt_freq2(void)
{
	return FPGA_TIMER_FREQUENCY;
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	/* TODO: determine if MMU needs to be enabled */
}
