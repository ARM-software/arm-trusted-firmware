/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mcucfg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void disable_scu(u_register_t mpidr)
{
	uintptr_t axi_config = 0;
	uint32_t axi_value;

	switch (mpidr & MPIDR_CLUSTER_MASK) {
	case 0x000:
		axi_config = (uintptr_t)&mt8183_mcucfg->mp0_axi_config;
		axi_value = MP0_ACINACTM;
		break;
	case 0x100:
		axi_config = (uintptr_t)&mt8183_mcucfg->mp2_axi_config;
		axi_value = MP2_ACINACTM;
		break;
	default:
		ERROR("%s: mpidr does not exist\n", __func__);
		panic();
	}
	mmio_setbits_32(axi_config, axi_value);
}

void enable_scu(u_register_t mpidr)
{
	uintptr_t axi_config = 0;
	uint32_t axi_value;

	switch (mpidr & MPIDR_CLUSTER_MASK) {
	case 0x000:
		axi_config = (uintptr_t)&mt8183_mcucfg->mp0_axi_config;
		axi_value = MP0_ACINACTM;
		break;
	case 0x100:
		axi_config = (uintptr_t)&mt8183_mcucfg->mp2_axi_config;
		axi_value = MP2_ACINACTM;
		break;
	default:
		ERROR("%s: mpidr does not exist\n", __func__);
		panic();
	}
	mmio_clrbits_32(axi_config, axi_value);
}
