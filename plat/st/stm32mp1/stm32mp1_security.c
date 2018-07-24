/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <mmio.h>
#include <stdint.h>
#include <stm32mp1_clk.h>
#include <stm32mp1_dt.h>
#include <stm32mp1_private.h>
#include <stm32mp1_rcc.h>
#include <tzc400.h>
#include "platform_def.h"

/*******************************************************************************
 * Initialize the TrustZone Controller.
 * Early initialization create only one region with full access to secure.
 * This setting is used before and during DDR initialization.
 ******************************************************************************/
static void early_init_tzc400(void)
{
	uint32_t rstsr, rst_standby;

	rstsr = mmio_read_32(RCC_BASE + RCC_MP_RSTSCLRR);

	/* No warning if return from (C)STANDBY */
	rst_standby = rstsr &
		(RCC_MP_RSTSCLRR_STDBYRSTF | RCC_MP_RSTSCLRR_CSTDBYRSTF);

	if (stm32mp1_clk_is_enabled(TZC1) && (rst_standby == 0U)) {
		WARN("TZC400 port 1 clock already enable\n");
	}

	if (stm32mp1_clk_is_enabled(TZC2) && (rst_standby == 0U)) {
		WARN("TZC400 port 2 clock already enable\n");
	}

	if (stm32mp1_clk_enable(TZC1) != 0) {
		ERROR("Cannot enable TZC1 clock\n");
		panic();
	}
	if (stm32mp1_clk_enable(TZC2) != 0) {
		ERROR("Cannot enable TZC2 clock\n");
		panic();
	}

	tzc400_init(STM32MP1_TZC_BASE);

	tzc400_disable_filters();

	/*
	 * Region 1 set to cover Non-Secure DRAM at 0x8000_0000. Apply the
	 * same configuration to all filters in the TZC.
	 */
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 1,
				STM32MP1_DDR_BASE,
				STM32MP1_DDR_BASE +
				(STM32MP1_DDR_MAX_SIZE - 1U),
				TZC_REGION_S_RDWR,
				TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_SDMMC_ID));

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	tzc400_enable_filters();
}

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void stm32mp1_arch_security_setup(void)
{
	early_init_tzc400();
}
