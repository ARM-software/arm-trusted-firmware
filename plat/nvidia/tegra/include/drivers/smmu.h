/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_H
#define SMMU_H

#include <lib/mmio.h>

#include <memctrl_v2.h>
#include <tegra_def.h>

#define SMMU_CBn_ACTLR				(0x4U)

/*******************************************************************************
 * SMMU Global Secure Aux. Configuration Register
 ******************************************************************************/
#define SMMU_GSR0_SECURE_ACR			0x10U
#define SMMU_GNSR_ACR				(SMMU_GSR0_SECURE_ACR + 0x400U)
#define SMMU_GSR0_PGSIZE_SHIFT			16U
#define SMMU_GSR0_PGSIZE_4K			(0U << SMMU_GSR0_PGSIZE_SHIFT)
#define SMMU_GSR0_PGSIZE_64K			(1U << SMMU_GSR0_PGSIZE_SHIFT)
#define SMMU_ACR_CACHE_LOCK_ENABLE_BIT		(1U << 26)

/*******************************************************************************
 * SMMU Global Aux. Control Register
 ******************************************************************************/
#define SMMU_CBn_ACTLR_CPRE_BIT			(1ULL << 1U)

void tegra_smmu_init(void);
uint32_t plat_get_num_smmu_devices(void);

#endif /* SMMU_H */
