/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <smmu.h>
#include <tegra_def.h>

#define BOARD_SYSTEM_FPGA_BASE		U(1)
#define BASE_CONFIG_SMMU_DEVICES	U(2)
#define MAX_NUM_SMMU_DEVICES		U(3)

static uint32_t tegra_misc_read_32(uint32_t off)
{
	return mmio_read_32((uintptr_t)TEGRA_MISC_BASE + off);
}

/*******************************************************************************
 * Handler to return the support SMMU devices number
 ******************************************************************************/
uint32_t plat_get_num_smmu_devices(void)
{
	uint32_t ret_num = MAX_NUM_SMMU_DEVICES;
	uint32_t board_revid = ((tegra_misc_read_32(MISCREG_EMU_REVID) >> \
							BOARD_SHIFT_BITS) & BOARD_MASK_BITS);

	if (board_revid == BOARD_SYSTEM_FPGA_BASE) {
		ret_num = BASE_CONFIG_SMMU_DEVICES;
	}

	return ret_num;
}
