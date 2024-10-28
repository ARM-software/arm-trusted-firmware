/*
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "socfpga_system_manager.h"

uint32_t intel_hps_get_jtag_id(void)
{
	uint32_t jtag_id = 0x00;

	jtag_id = (mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_4)));

	INFO("%s: JTAG ID: 0x%x\n", __func__, jtag_id);

	return jtag_id;
}

/* Check for Agilex5 SM4 */
bool is_agilex5_A5F0(void)
{
	return ((intel_hps_get_jtag_id() & JTAG_ID_MASK) == A5F0_JTAG_ID);
}
