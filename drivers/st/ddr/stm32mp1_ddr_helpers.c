/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <platform_def.h>
#include <stm32mp1_ddr_helpers.h>
#include <stm32mp1_rcc.h>

void ddr_enable_clock(void)
{
	mmio_setbits_32(RCC_BASE + RCC_DDRITFCR,
			RCC_DDRITFCR_DDRC1EN |
			RCC_DDRITFCR_DDRC2EN |
			RCC_DDRITFCR_DDRPHYCEN |
			RCC_DDRITFCR_DDRPHYCAPBEN |
			RCC_DDRITFCR_DDRCAPBEN);
}
