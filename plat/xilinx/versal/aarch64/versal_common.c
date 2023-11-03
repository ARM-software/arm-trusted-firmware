/*
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <plat_common.h>
#include <plat_ipi.h>
#include <plat_private.h>
#include <pm_api_sys.h>
#include <versal_def.h>

uint32_t platform_id, platform_version;

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_versal_mmap[] = {
	MAP_REGION_FLAT(DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRF_BASE, CRF_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PLAT_ARM_CCI_BASE, PLAT_ARM_CCI_SIZE, MT_DEVICE | MT_RW |
			MT_SECURE),
	{ 0 }
};

const mmap_region_t *plat_get_mmap(void)
{
	return plat_versal_mmap;
}

static void versal_print_platform_name(void)
{
	NOTICE("TF-A running on %s\n", PLATFORM_NAME);
}

void versal_config_setup(void)
{
	/* Configure IPI data for versal */
	versal_ipi_config_table_init();

	versal_print_platform_name();

	generic_delay_timer_init();
}

uint32_t plat_get_syscnt_freq2(void)
{
	return VERSAL_CPU_CLOCK;
}

void board_detection(void)
{
	uint32_t plat_info[2];

	if (pm_get_chipid(plat_info) != PM_RET_SUCCESS) {
		/* If the call is failed we cannot proceed with further
		 * setup. TF-A to panic in this situation.
		 */
		NOTICE("Failed to read the chip information");
		panic();
	}

	platform_id = FIELD_GET(PLATFORM_MASK, plat_info[1]);
	platform_version = FIELD_GET(PLATFORM_VERSION_MASK, plat_info[1]);
}

uint32_t get_uart_clk(void)
{
	return UART_CLOCK;
}
