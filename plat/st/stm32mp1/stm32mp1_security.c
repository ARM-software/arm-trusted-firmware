/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
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

static unsigned int region_nb;

static void init_tzc400_begin(void)
{
	tzc400_init(STM32MP1_TZC_BASE);
	tzc400_disable_filters();

	region_nb = 1U;
}

static void init_tzc400_end(unsigned int action)
{
	tzc400_set_action(action);
	tzc400_enable_filters();
}

static void tzc400_add_region(unsigned long long region_base,
			      unsigned long long region_top, bool sec)
{
	unsigned int sec_attr;
	unsigned int nsaid_permissions;

	if (sec) {
		sec_attr = TZC_REGION_S_RDWR;
		nsaid_permissions = 0;
	} else {
		sec_attr = TZC_REGION_S_NONE;
		nsaid_permissions = TZC_REGION_NSEC_ALL_ACCESS_RDWR;
	}

	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, region_nb, region_base,
				region_top, sec_attr, nsaid_permissions);

	region_nb++;
}

/*******************************************************************************
 * Initialize the TrustZone Controller. Configure Region 0 with Secure RW access
 * and allow Non-Secure masters full access.
 ******************************************************************************/
static void init_tzc400(void)
{
	unsigned long long region_base, region_top;
	unsigned long long ddr_base = STM32MP_DDR_BASE;
	unsigned long long ddr_ns_size =
		(unsigned long long)stm32mp_get_ddr_ns_size();
	unsigned long long ddr_ns_top = ddr_base + (ddr_ns_size - 1U);
	unsigned long long ddr_top __unused;

	init_tzc400_begin();

	/*
	 * Region 1 set to cover all non-secure DRAM at 0xC000_0000. Apply the
	 * same configuration to all filters in the TZC.
	 */
	region_base = ddr_base;
	region_top = ddr_ns_top;
	tzc400_add_region(region_base, region_top, false);

#ifdef AARCH32_SP_OPTEE
	/* Region 2 set to cover all secure DRAM. */
	region_base = region_top + 1U;
	region_top += STM32MP_DDR_S_SIZE;
	tzc400_add_region(region_base, region_top, true);

	ddr_top = STM32MP_DDR_BASE + dt_get_ddr_size() - 1U;
	if (region_top < ddr_top) {
		/* Region 3 set to cover non-secure memory DRAM after BL32. */
		region_base = region_top + 1U;
		region_top = ddr_top;
		tzc400_add_region(region_base, region_top, false);
	}
#endif

	/*
	 * Raise an interrupt (secure FIQ) if a NS device tries to access
	 * secure memory
	 */
	init_tzc400_end(TZC_ACTION_INT);
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

	init_tzc400_begin();

	/* Region 1 set to cover Non-Secure DRAM at 0xC000_0000 */
	tzc400_configure_region(STM32MP1_FILTER_BIT_ALL, 1,
				STM32MP_DDR_BASE,
				STM32MP_DDR_BASE +
				(STM32MP_DDR_MAX_SIZE - 1U),
				TZC_REGION_S_NONE,
				TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_A7_ID) |
				TZC_REGION_ACCESS_RDWR(STM32MP1_TZC_SDMMC_ID));

	/* Raise an exception if a NS device tries to access secure memory */
	init_tzc400_end(TZC_ACTION_ERR);
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
