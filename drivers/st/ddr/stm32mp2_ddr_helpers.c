/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <platform_def.h>

void ddr_sub_system_clk_init(void)
{
	mmio_write_32(stm32mp_rcc_base() + RCC_DDRCPCFGR,
		      RCC_DDRCPCFGR_DDRCPEN | RCC_DDRCPCFGR_DDRCPLPEN);
}
