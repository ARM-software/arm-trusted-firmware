/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dt-bindings/clock/stm32mp1-clks.h>
#include <errno.h>
#include <mmio.h>
#include <platform_def.h>
#include <stm32mp1_clk.h>
#include <stm32mp1_context.h>

#define TAMP_BOOT_ITF_BACKUP_REG_ID	U(20)
#define TAMP_BOOT_ITF_MASK		U(0x0000FF00)
#define TAMP_BOOT_ITF_SHIFT		8

int stm32_save_boot_interface(uint32_t interface, uint32_t instance)
{
	uint32_t tamp_clk_off = 0;
	uint32_t bkpr_itf_idx = tamp_bkpr(TAMP_BOOT_ITF_BACKUP_REG_ID);

	if (!stm32mp1_clk_is_enabled(RTCAPB)) {
		tamp_clk_off = 1;
		if (stm32mp1_clk_enable(RTCAPB) != 0) {
			return -EINVAL;
		}
	}

	mmio_clrsetbits_32(bkpr_itf_idx,
			   TAMP_BOOT_ITF_MASK,
			   ((interface << 4) | (instance & 0xFU)) <<
			   TAMP_BOOT_ITF_SHIFT);

	if (tamp_clk_off != 0U) {
		if (stm32mp1_clk_disable(RTCAPB) != 0) {
			return -EINVAL;
		}
	}

	return 0;
}
