/*
 * Copyright (c) 2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <stdint.h>

#include <stm32mp_common.h>

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	stm32mp_setup_early_console();
}

void bl2_platform_setup(void)
{
}

void bl2_el3_plat_arch_setup(void)
{
}
