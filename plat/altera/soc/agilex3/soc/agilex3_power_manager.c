/*
 * Copyright (c) 2022-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "agilex3_power_manager.h"
#include "socfpga_reset_manager.h"

static int wait_verify_fsm(uint16_t timeout, uint32_t peripheral_handoff)
{
	uint32_t data = 0;
	uint32_t count = 0;
	uint32_t pgstat = 0;

	/* Wait FSM ready */
	do {
		data = mmio_read_32(AGX3_PWRMGR(PSS_PGSTAT));
		count++;
		if (count >= 1000) {
			return -ETIMEDOUT;
		}

	} while (AGX3_PWRMGR_PSS_STAT_BUSY(data) == AGX3_PWRMGR_PSS_STAT_BUSY_E_BUSY);

	/* Verify PSS SRAM power gated */
	pgstat = mmio_read_32(AGX3_PWRMGR(PSS_PGSTAT));
	if (pgstat != (AGX3_PWRMGR_PSS_PGEN_OUT(peripheral_handoff))) {
		return AGX3_PWRMGR_HANDOFF_PERIPHERAL;
	}

	return 0;
}

static int pss_sram_power_off(handoff *hoff_ptr)
{
	int ret = 0;
	uint32_t peripheral_handoff = 0;

	/* Get PSS SRAM handoff data */
	peripheral_handoff = hoff_ptr->peripheral_pwr_gate_array;

	/* Enable firewall for PSS SRAM */
	mmio_write_32(AGX3_PWRMGR(PSS_FWENCTL),
			AGX3_PWRMGR_PSS_FWEN(peripheral_handoff));

	/* Wait */
	udelay(1);

	/* Power gating PSS SRAM */
	mmio_write_32(AGX3_PWRMGR(PSS_PGENCTL),
			AGX3_PWRMGR_PSS_PGEN(peripheral_handoff));

	ret = wait_verify_fsm(1000, peripheral_handoff);

	return ret;
}

void config_pwrmgr_handoff(handoff *hoff_ptr)
{
	int ret = 0;

	switch (hoff_ptr->peripheral_pwr_gate_magic) {
	case HANDOFF_MAGIC_PERIPHERAL:
		ret = pss_sram_power_off(hoff_ptr);
		break;
	default:
		break;
	}

	if (ret != 0) {
		ERROR("Config PwrMgr handoff failed. error %d\n", ret);
		assert(false);
	}
}
