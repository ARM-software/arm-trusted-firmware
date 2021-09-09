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
#include <dt-bindings/soc/stm32mp15-tzc400.h>
#include <lib/mmio.h>

static unsigned int region_nb;

static void init_tzc400_begin(unsigned int region0_attr)
{
	tzc400_init(STM32MP1_TZC_BASE);
	tzc400_disable_filters();

	/* Region 0 set to cover all DRAM at 0xC000_0000 */
	tzc400_configure_region0(region0_attr, 0);

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

	init_tzc400_begin(TZC_REGION_S_NONE);

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

	/* Region 0 set to cover all DRAM secure at 0xC000_0000 */
	init_tzc400_begin(TZC_REGION_S_RDWR);

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
