/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>

#include <smmu.h>
#include <tegra_private.h>

extern void memcpy16(void *dest, const void *src, unsigned int length);

/* SMMU IDs currently supported by the driver */
enum {
	TEGRA_SMMU0 = 0U,
	TEGRA_SMMU1,
	TEGRA_SMMU2
};

static uint32_t tegra_smmu_read_32(uint32_t smmu_id, uint32_t off)
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

static void tegra_smmu_write_32(uint32_t smmu_id,
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

#define SMMU_NUM_CONTEXTS		64U
#define SMMU_CONTEXT_BANK_MAX_IDX	64U

/*
 * Init SMMU during boot or "System Suspend" exit
 */
void tegra_smmu_init(void)
{
	uint32_t val, cb_idx, smmu_id, ctx_base;
	uint32_t smmu_counter = plat_get_num_smmu_devices();

	for (smmu_id = 0U; smmu_id < smmu_counter; smmu_id++) {
		/* Program the SMMU pagesize and reset CACHE_LOCK bit */
		val = tegra_smmu_read_32(smmu_id, SMMU_GSR0_SECURE_ACR);
		val |= SMMU_GSR0_PGSIZE_64K;
		val &= (uint32_t)~SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GSR0_SECURE_ACR, val);

		/* reset CACHE LOCK bit for NS Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GNSR_ACR);
		val &= (uint32_t)~SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GNSR_ACR, val);

		/* disable TCU prefetch for all contexts */
		ctx_base = (SMMU_GSR0_PGSIZE_64K * SMMU_NUM_CONTEXTS)
				+ SMMU_CBn_ACTLR;
		for (cb_idx = 0; cb_idx < SMMU_CONTEXT_BANK_MAX_IDX; cb_idx++) {
			val = tegra_smmu_read_32(smmu_id,
				ctx_base + (SMMU_GSR0_PGSIZE_64K * cb_idx));
			val &= (uint32_t)~SMMU_CBn_ACTLR_CPRE_BIT;
			tegra_smmu_write_32(smmu_id, ctx_base +
				(SMMU_GSR0_PGSIZE_64K * cb_idx), val);
		}

		/* set CACHE LOCK bit for NS Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GNSR_ACR);
		val |= (uint32_t)SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GNSR_ACR, val);

		/* set CACHE LOCK bit for S Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GSR0_SECURE_ACR);
		val |= (uint32_t)SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GSR0_SECURE_ACR, val);
	}
}
