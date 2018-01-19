/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <console.h>
#include <generic_delay_timer.h>
#include <plat_arm.h>
#include <platform.h>

#pragma weak bl2_el3_early_platform_setup
#pragma weak bl2_el3_plat_arch_setup
#pragma weak bl2_el3_plat_prepare_exit

static meminfo_t bl2_el3_tzram_layout;

/*
 * Perform arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void arm_bl2_el3_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_ARM_BOOT_UART_BASE, PLAT_ARM_BOOT_UART_CLK_IN_HZ,
			ARM_CONSOLE_BAUDRATE);

	/*
	 * Allow BL2 to see the whole Trusted RAM. This is determined
	 * statically since we cannot rely on BL1 passing this information
	 * in the BL2_AT_EL3 case.
	 */
	bl2_el3_tzram_layout.total_base = ARM_BL_RAM_BASE;
	bl2_el3_tzram_layout.total_size = ARM_BL_RAM_SIZE;

	/* Initialise the IO layer and register platform IO devices */
	plat_arm_io_setup();
}

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	arm_bl2_el3_early_platform_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();
	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_arm_interconnect_enter_coherency();

	generic_delay_timer_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void arm_bl2_el3_plat_arch_setup(void)
{
	arm_setup_page_tables(bl2_el3_tzram_layout.total_base,
			      bl2_el3_tzram_layout.total_size,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			      );

#ifdef AARCH32
	enable_mmu_secure(0);
#else
	enable_mmu_el3(0);
#endif
}

void bl2_el3_plat_arch_setup(void)
{
	arm_bl2_el3_plat_arch_setup();
}

void bl2_el3_plat_prepare_exit(void)
{
}
