/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ls_def.h>
#include <bl_common.h>
#include <console.h>
#include <platform_def.h>
#include "platform_tsp.h"
#include <plat_arm.h>
#include <gicv2.h>
#include <debug.h>
#define BL32_END (unsigned long)(&__BL32_END__)

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak tsp_early_platform_setup
#pragma weak tsp_platform_setup
#pragma weak tsp_plat_arch_setup

const unsigned int g0_interrupt_array1[] = {
	9
};

const gicv2_driver_data_t arm_gic_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array1),
	.g0_interrupt_array = g0_interrupt_array1,
};

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
void arm_tsp_early_platform_setup(void)
{
	/*
	 * Initialize a different console than already in use to display
	 * messages from TSP
	 */
	console_init(PLAT_LS1043_UART2_BASE, PLAT_LS1043_UART_CLOCK,
			PLAT_LS1043_UART_BAUDRATE);
	NOTICE(FIRMWARE_WELCOME_STR_LS1043_BL32);
}

void tsp_early_platform_setup(void)
{
	arm_tsp_early_platform_setup();
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
	gicv2_driver_init(&arm_gic_data);
#if 0
#ifdef HH_DEBUG
	memcpy(0xfc000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0xfc000000\n");
	memcpy(0xec000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0xec000000\n");
	memcpy(0xdc000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0xdc000000\n");
	memcpy(0xcc000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0xcc000000\n");
	memcpy(0xbc000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0xbc000000\n");
	memcpy(0xac000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0xac000000\n");
	memcpy(0x9c000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0x9c000000\n");
	memcpy(0x8c000000, BL32_BASE, TSP_SEC_MEM_SIZE);
	VERBOSE("Successful for 0x8c000000\n");
#endif
#endif
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU
 ******************************************************************************/
void tsp_plat_arch_setup(void)
{
	arm_setup_page_tables(BL32_BASE,
			      (BL32_END - BL32_BASE),
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			      );
	enable_mmu_el1(0);
}
