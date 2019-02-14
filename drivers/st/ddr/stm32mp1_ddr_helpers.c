/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <drivers/st/stm32mp1_ddr_helpers.h>
#include <lib/mmio.h>

void ddr_enable_clock(void)
{
	stm32mp1_clk_rcc_regs_lock();

	mmio_setbits_32(stm32mp_rcc_base() + RCC_DDRITFCR,
			RCC_DDRITFCR_DDRC1EN |
			RCC_DDRITFCR_DDRC2EN |
			RCC_DDRITFCR_DDRPHYCEN |
			RCC_DDRITFCR_DDRPHYCAPBEN |
			RCC_DDRITFCR_DDRCAPBEN);

	stm32mp1_clk_rcc_regs_unlock();
}
