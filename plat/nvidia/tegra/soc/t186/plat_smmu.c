/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>

#include <smmu.h>
#include <tegra_def.h>
#include <tegra_mc_def.h>

#define MAX_NUM_SMMU_DEVICES	U(1)

/*******************************************************************************
 * Handler to return the support SMMU devices number
 ******************************************************************************/
uint32_t plat_get_num_smmu_devices(void)
{
	return MAX_NUM_SMMU_DEVICES;
}
