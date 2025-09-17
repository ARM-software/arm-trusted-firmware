/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <bl32/tsp/platform_tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#if TRANSFER_LIST && MEASURED_BOOT
#include <event_measure.h>
#include <event_print.h>
#endif
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak tsp_early_platform_setup
#pragma weak tsp_platform_setup
#pragma weak tsp_plat_arch_setup

#define MAP_BL_TSP_TOTAL	MAP_REGION_FLAT(			\
					BL32_BASE,			\
					BL32_END - BL32_BASE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_FW_HANDOFF		MAP_REGION_FLAT(			\
					PLAT_ARM_EL3_FW_HANDOFF_BASE,	\
					PLAT_ARM_FW_HANDOFF_SIZE,	\
					MT_MEMORY | MT_RO | MT_SECURE)

struct transfer_list_header *secure_tl __unused;

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
static console_t arm_tsp_runtime_console;

void arm_tsp_early_platform_setup(u_register_t arg0, u_register_t arg1,
			      u_register_t arg2, u_register_t arg3)
{
#if TRANSFER_LIST
	secure_tl = (struct transfer_list_header *)arg3;
	assert(secure_tl != NULL);

	if (transfer_list_check_header(secure_tl) == TL_OPS_NON) {
		ERROR("Invalid transfer list received");
		transfer_list_dump(secure_tl);
		panic();
	}
#endif

	/*
	 * Initialize a different console than already in use to display
	 * messages from TSP
	 */
	int rc = console_pl011_register(PLAT_ARM_TSP_UART_BASE,
					PLAT_ARM_TSP_UART_CLK_IN_HZ,
					ARM_CONSOLE_BAUDRATE,
					&arm_tsp_runtime_console);
	if (rc == 0) {
		panic();
	}

	console_set_scope(&arm_tsp_runtime_console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}

void tsp_early_platform_setup(u_register_t arg0, u_register_t arg1,
			      u_register_t arg2, u_register_t arg3)
{
	arm_tsp_early_platform_setup(arg0, arg1, arg2, arg3);
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
	struct transfer_list_entry *te __unused;

	/*
	 * On GICv2 the driver must be initialised before calling the plat_ic_*
	 * functions as they need the data structures. Higher versions don't.
	 */
#if USE_GIC_DRIVER == 2
	gic_init(plat_my_core_pos());
#endif

#if TRANSFER_LIST && MEASURED_BOOT
	te = transfer_list_find(secure_tl, TL_TAG_TPM_EVLOG);
	assert(te != NULL);

	/*
	 * Note the actual log is offset 4-bytes from the start of entry data, the
	 * first bytes are reserved.
	 */
	event_log_dump(transfer_list_entry_data(te) + U(4), te->data_size - U(4));
#endif
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the MMU
 ******************************************************************************/
void tsp_plat_arch_setup(void)
{
#if USE_COHERENT_MEM
	/* Ensure ARM platforms don't use coherent memory in TSP */
	assert((BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE) == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL_TSP_TOTAL,
		ARM_MAP_BL_RO,
#if TRANSFER_LIST
		MAP_FW_HANDOFF,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());
	enable_mmu_el1(0);

#if PLAT_RO_XLAT_TABLES
	arm_xlat_make_tables_readonly();
#endif
}
