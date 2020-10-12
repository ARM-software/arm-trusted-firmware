/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <lib/mmio.h>
#include <plat/common/platform.h>

#if MSS_SUPPORT
#include <mss_mem.h>

#ifdef PM_TRACE_ENABLE
#include <plat_pm_trace.h>

/* core trace APIs */
core_trace_func funcTbl[PLATFORM_CORE_COUNT] = {
	pm_core_0_trace,
	pm_core_1_trace,
	pm_core_2_trace,
	pm_core_3_trace};

/*****************************************************************************
 * pm_core0_trace
 * pm_core1_trace
 * pm_core2_trace
 * pm_core_3trace
 *
 * This functions set trace info into core cyclic trace queue in MSS SRAM
 * memory space
 *****************************************************************************
 */
void pm_core_0_trace(unsigned int trace)
{
	unsigned int current_position_core_0 =
			mmio_read_32(AP_MSS_ATF_CORE_0_CTRL_BASE);
	mmio_write_32((AP_MSS_ATF_CORE_0_INFO_BASE  +
		     (current_position_core_0 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     mmio_read_32(AP_MSS_TIMER_BASE));
	mmio_write_32((AP_MSS_ATF_CORE_0_INFO_TRACE +
		     (current_position_core_0 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     trace);
	mmio_write_32(AP_MSS_ATF_CORE_0_CTRL_BASE,
		     ((current_position_core_0 + 1) &
		     AP_MSS_ATF_TRACE_SIZE_MASK));
}

void pm_core_1_trace(unsigned int trace)
{
	unsigned int current_position_core_1 =
			mmio_read_32(AP_MSS_ATF_CORE_1_CTRL_BASE);
	mmio_write_32((AP_MSS_ATF_CORE_1_INFO_BASE +
		     (current_position_core_1 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     mmio_read_32(AP_MSS_TIMER_BASE));
	mmio_write_32((AP_MSS_ATF_CORE_1_INFO_TRACE +
		     (current_position_core_1 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     trace);
	mmio_write_32(AP_MSS_ATF_CORE_1_CTRL_BASE,
		     ((current_position_core_1 + 1) &
		     AP_MSS_ATF_TRACE_SIZE_MASK));
}

void pm_core_2_trace(unsigned int trace)
{
	unsigned int current_position_core_2 =
			mmio_read_32(AP_MSS_ATF_CORE_2_CTRL_BASE);
	mmio_write_32((AP_MSS_ATF_CORE_2_INFO_BASE +
		     (current_position_core_2 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     mmio_read_32(AP_MSS_TIMER_BASE));
	mmio_write_32((AP_MSS_ATF_CORE_2_INFO_TRACE +
		     (current_position_core_2 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     trace);
	mmio_write_32(AP_MSS_ATF_CORE_2_CTRL_BASE,
		     ((current_position_core_2 + 1) &
		     AP_MSS_ATF_TRACE_SIZE_MASK));
}

void pm_core_3_trace(unsigned int trace)
{
	unsigned int current_position_core_3 =
			mmio_read_32(AP_MSS_ATF_CORE_3_CTRL_BASE);
	mmio_write_32((AP_MSS_ATF_CORE_3_INFO_BASE +
		     (current_position_core_3 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     mmio_read_32(AP_MSS_TIMER_BASE));
	mmio_write_32((AP_MSS_ATF_CORE_3_INFO_TRACE +
		     (current_position_core_3 * AP_MSS_ATF_CORE_ENTRY_SIZE)),
		     trace);
	mmio_write_32(AP_MSS_ATF_CORE_3_CTRL_BASE,
		     ((current_position_core_3 + 1) &
		     AP_MSS_ATF_TRACE_SIZE_MASK));
}
#endif /* PM_TRACE_ENABLE */
#endif /* MSS_SUPPORT */
