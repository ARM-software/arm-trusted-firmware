/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/tzc400.h>
#include <drivers/st/stm32mp1_clk.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <lib/mmio.h>

#define TZC_REGION_NSEC_ALL_ACCESS_RDWR \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_A7_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_GPU_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_LCD_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_MDMA_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_M4_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_DMA_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_USB_HOST_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_USB_OTG_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_SDMMC_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_ETH_ID) | \
	TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_DAP_ID)

/*******************************************************************************
 * Initialize the TrustZone Controller. Configure Region 0 with Secure RW access
 * and allow Non-Secure masters full access.
 ******************************************************************************/
static void init_tzc400(void)
{
	unsigned long long region_base, region_top;
	unsigned long long ddr_base = STM32MP_DDR_BASE;
	unsigned long long ddr_size = (unsigned long long)dt_get_ddr_size();
	unsigned long long ddr_top = ddr_base + (ddr_size - 1U);

	tzc400_init(STM32MP1_TZC_BASE);

	tzc400_disable_filters();

#ifdef AARCH32_SP_OPTEE
	/*
	 * Region 1 set to cover all non-secure DRAM at 0xC000_0000. Apply the
	 * same configuration to all filters in the TZC.
	 */
	region_base = ddr_base;
	region_top = ddr_top - STM32MP_DDR_S_SIZE - STM32MP_DDR_SHMEM_SIZE;
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 1,
				region_base,
				region_top,
				TZC_REGION_S_NONE,
				TZC_REGION_NSEC_ALL_ACCESS_RDWR);

	/* Region 2 set to cover all secure DRAM. */
	region_base = region_top + 1U;
	region_top = ddr_top - STM32MP_DDR_SHMEM_SIZE;
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 2,
				region_base,
				region_top,
				TZC_REGION_S_RDWR,
				0);

	/* Region 3 set to cover non-secure shared memory DRAM. */
	region_base = region_top + 1U;
	region_top = ddr_top;
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 3,
				region_base,
				region_top,
				TZC_REGION_S_NONE,
				TZC_REGION_NSEC_ALL_ACCESS_RDWR);
#else
	/*
	 * Region 1 set to cover all DRAM at 0xC000_0000. Apply the
	 * same configuration to all filters in the TZC.
	 */
	region_base = ddr_base;
	region_top = ddr_top;
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 1,
				region_base,
				region_top,
				TZC_REGION_S_NONE,
				TZC_REGION_NSEC_ALL_ACCESS_RDWR);
#endif

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
	stm32mp_clk_enable(TZC1);
	stm32mp_clk_enable(TZC2);

	tzc400_init(STM32MP1_TZC_BASE);

	tzc400_disable_filters();

	/* Region 1 set to cover Non-Secure DRAM at 0xC000_0000 */
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 1,
				STM32MP_DDR_BASE,
				STM32MP_DDR_BASE +
				(STM32MP_DDR_MAX_SIZE - 1U),
				TZC_REGION_S_NONE,
				TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_A7_ID) |
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
