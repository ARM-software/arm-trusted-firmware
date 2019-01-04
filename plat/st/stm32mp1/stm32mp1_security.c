/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/tzc400.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <lib/mmio.h>

#include <stm32mp1_dt.h>
#include <stm32mp1_private.h>

/*******************************************************************************
 * Initialize the TrustZone Controller. Configure Region 0 with Secure RW access
 * and allow Non-Secure masters full access.
 ******************************************************************************/
static void init_tzc400(void)
{
	unsigned long long region_base, region_top;
	unsigned long long ddr_base = STM32MP1_DDR_BASE;
	unsigned long long ddr_size = (unsigned long long)dt_get_ddr_size();

	tzc400_init(STM32MP1_TZC_BASE);

	tzc400_disable_filters();

	/* Region 1 set to cover all DRAM at 0xC000_0000. Apply the
	 * same configuration to all filters in the TZC.
	 */
	region_base = ddr_base;
	region_top = ddr_base + (ddr_size - 1U);
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 1,
			region_base,
			region_top,
			TZC_REGION_S_RDWR,
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_A7_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_GPU_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_LCD_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_MDMA_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_DMA_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_USB_HOST_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_USB_OTG_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_SDMMC_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_ETH_ID) |
			TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_DAP_ID));

	/* Raise an exception if a NS device tries to access secure memory */
	tzc400_set_action(TZC_ACTION_ERR);

	tzc400_enable_filters();
}

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

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void stm32mp1_security_setup(void)
{
	init_tzc400();
}
