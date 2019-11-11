/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>
#include <platform_def.h>

#include "fpga_private.h"

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	fpga_console_init();
	/*
	 * TODO: implement any extra early platform setup before jumping to BL33
	 * payload
	 */
}

void bl31_plat_arch_setup(void)
{
}

void bl31_platform_setup(void)
{
	/* TODO: initialize GIC and timer using the specifications of the FPGA image */
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	/*
	 * TODO: return entry_point_info_t struct containing information about the
	 *       BL33 payload, which will run in EL2NS mode.
	 */
	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	/*
	 * TODO: return the frequency of the System Counter as configured by the
	 * FPGA image
	 */
	return 0;
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	/* TODO: determine if MMU needs to be enabled */
}
