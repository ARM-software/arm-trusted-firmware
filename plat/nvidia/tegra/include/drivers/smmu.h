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
#define SMMU_ACR_CACHE_LOCK_ENABLE_BIT		(1ULL << 26U)
#define SMMU_GSR0_PER				(0x20200U)

/*******************************************************************************
 * SMMU Global Aux. Control Register
 ******************************************************************************/
#define SMMU_CBn_ACTLR_CPRE_BIT			(1ULL << 1U)

/* SMMU IDs currently supported by the driver */
enum {
	TEGRA_SMMU0 = 0U,
	TEGRA_SMMU1 = 1U,
	TEGRA_SMMU2 = 2U
};

static inline uint32_t tegra_smmu_read_32(uint32_t smmu_id, uint32_t off)
{
	uint32_t ret = 0U;

#if defined(TEGRA_SMMU0_BASE)
	if (smmu_id == TEGRA_SMMU0) {
		ret = mmio_read_32(TEGRA_SMMU0_BASE + (uint64_t)off);
	}
#endif

#if defined(TEGRA_SMMU1_BASE)
	if (smmu_id == TEGRA_SMMU1) {
		ret = mmio_read_32(TEGRA_SMMU1_BASE + (uint64_t)off);
	}
#endif

#if defined(TEGRA_SMMU2_BASE)
	if (smmu_id == TEGRA_SMMU2) {
		ret = mmio_read_32(TEGRA_SMMU2_BASE + (uint64_t)off);
	}
#endif

	return ret;
}

static inline void tegra_smmu_write_32(uint32_t smmu_id,
			uint32_t off, uint32_t val)
{
#if defined(TEGRA_SMMU0_BASE)
	if (smmu_id == TEGRA_SMMU0) {
		mmio_write_32(TEGRA_SMMU0_BASE + (uint64_t)off, val);
	}
#endif

#if defined(TEGRA_SMMU1_BASE)
	if (smmu_id == TEGRA_SMMU1) {
		mmio_write_32(TEGRA_SMMU1_BASE + (uint64_t)off, val);
	}
#endif

#if defined(TEGRA_SMMU2_BASE)
	if (smmu_id == TEGRA_SMMU2) {
		mmio_write_32(TEGRA_SMMU2_BASE + (uint64_t)off, val);
	}
#endif
}

void tegra_smmu_init(void);
void tegra_smmu_verify(void);
uint32_t plat_get_num_smmu_devices(void);

#endif /* SMMU_H */
