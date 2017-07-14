/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_def.h>
#include <bl_common.h>
#include <console.h>
#include <plat_arm.h>
#include <platform_def.h>
#include <string.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl2u_platform_setup
#pragma weak bl2u_early_platform_setup
#pragma weak bl2u_plat_arch_setup

/*
 * Perform ARM standard platform setup for BL2U
 */
void arm_bl2u_platform_setup(void)
{
	/* Initialize the secure environment */
	plat_arm_security_setup();
}

void bl2u_platform_setup(void)
{
	arm_bl2u_platform_setup();
}

void arm_bl2u_early_platform_setup(meminfo_t *mem_layout, void *plat_info)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_ARM_BOOT_UART_BASE, PLAT_ARM_BOOT_UART_CLK_IN_HZ,
			ARM_CONSOLE_BAUDRATE);
}

/*******************************************************************************
 * BL1 can pass platform dependent information to BL2U in x1.
 * In case of ARM CSS platforms x1 contains SCP_BL2U image info.
 * In case of ARM FVP platforms x1 is not used.
 * In both cases, x0 contains the extents of the memory available to BL2U
 ******************************************************************************/
void bl2u_early_platform_setup(meminfo_t *mem_layout, void *plat_info)
{
	arm_bl2u_early_platform_setup(mem_layout, plat_info);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 * The memory that is used by BL2U is only mapped.
 ******************************************************************************/
void arm_bl2u_plat_arch_setup(void)
{
	arm_setup_page_tables(BL2U_BASE,
			      BL31_LIMIT,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      ,
			      BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
		);
#ifdef AARCH32
	enable_mmu_secure(0);
#else
	enable_mmu_el1(0);
#endif
}

void bl2u_plat_arch_setup(void)
{
	arm_bl2u_plat_arch_setup();
}
