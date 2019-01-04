/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <endian.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "soc_tzasc.h"

int tzc380_set_region(unsigned int tzasc_base, unsigned int region_id,
		unsigned int enabled, unsigned int low_addr,
		unsigned int high_addr, unsigned int size,
		unsigned int security, unsigned int subreg_disable_mask)
{
	unsigned int reg;
	unsigned int reg_base;
	unsigned int attr_value;

	reg_base = (tzasc_base + TZASC_REGIONS_REG + (region_id << 4));

	if (region_id == 0) {
		reg = (reg_base + TZASC_REGION_ATTR_OFFSET);
		mmio_write_32((uintptr_t)reg, ((security & 0xF) << 28));
	} else {
		reg = reg_base + TZASC_REGION_LOWADDR_OFFSET;
		mmio_write_32((uintptr_t)reg,
				(low_addr & TZASC_REGION_LOWADDR_MASK));

		reg = reg_base + TZASC_REGION_HIGHADDR_OFFSET;
		mmio_write_32((uintptr_t)reg, high_addr);

		reg = reg_base + TZASC_REGION_ATTR_OFFSET;
		attr_value = ((security & 0xF) << 28) |
			((subreg_disable_mask & 0xFF) << 8) |
			((size & 0x3F) << 1) | (enabled & 0x1);
		mmio_write_32((uintptr_t)reg, attr_value);

	}
	return 0;
}

int tzc380_setup(void)
{
	int reg_id = 0;

	INFO("Configuring TZASC-380\n");

	/*
	 * Configure CCI control override register to terminate all barrier
	 * transactions
	 */
	mmio_write_32(PLAT_LS1043_CCI_BASE, CCI_TERMINATE_BARRIER_TX);

	/* Configure CSU secure access register to disable TZASC bypass mux */
	mmio_write_32((uintptr_t)(CONFIG_SYS_FSL_CSU_ADDR +
				CSU_SEC_ACCESS_REG_OFFSET),
			bswap32(TZASC_BYPASS_MUX_DISABLE));

	for (reg_id = 0; reg_id < MAX_NUM_TZC_REGION; reg_id++) {
		tzc380_set_region(CONFIG_SYS_FSL_TZASC_ADDR,
				reg_id,
				tzc380_reg_list[reg_id].enabled,
				tzc380_reg_list[reg_id].low_addr,
				tzc380_reg_list[reg_id].high_addr,
				tzc380_reg_list[reg_id].size,
				tzc380_reg_list[reg_id].secure,
				tzc380_reg_list[reg_id].sub_mask);
	}

	return 0;
}
