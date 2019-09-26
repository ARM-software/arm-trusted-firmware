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
#include <tegra_platform.h>
#include <tegra_private.h>

extern void memcpy16(void *dest, const void *src, unsigned int length);

#define SMMU_NUM_CONTEXTS		64U
#define SMMU_CONTEXT_BANK_MAX_IDX	64U

#define MISMATCH_DETECTED		0x55AA55AAU

/*
 * Init SMMU during boot or "System Suspend" exit
 */
void tegra_smmu_init(void)
{
	uint32_t val, cb_idx, smmu_id, ctx_base;
	uint32_t num_smmu_devices = plat_get_num_smmu_devices();

	for (smmu_id = 0U; smmu_id < num_smmu_devices; smmu_id++) {
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
		for (cb_idx = 0U; cb_idx < SMMU_CONTEXT_BANK_MAX_IDX; cb_idx++) {
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

/*
 * Verify SMMU settings have not been altered during boot
 */
void tegra_smmu_verify(void)
{
	uint32_t cb_idx, ctx_base, smmu_id, val;
	uint32_t num_smmu_devices = plat_get_num_smmu_devices();
	uint32_t mismatch = 0U;

	for (smmu_id = 0U; smmu_id < num_smmu_devices; smmu_id++) {
		/* check PGSIZE_64K bit inr S Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GSR0_SECURE_ACR);
		if (0U == (val & SMMU_GSR0_PGSIZE_64K)) {
			ERROR("%s: PGSIZE_64K Mismatch - smmu_id=%d, GSR0_SECURE_ACR=%x\n",
				__func__, smmu_id, val);
			mismatch = MISMATCH_DETECTED;
		}

		/* check CACHE LOCK bit in S Aux. Config. Register */
		if (0U == (val & SMMU_ACR_CACHE_LOCK_ENABLE_BIT)) {
			ERROR("%s: CACHE_LOCK Mismatch - smmu_id=%d, GSR0_SECURE_ACR=%x\n",
				__func__, smmu_id, val);
			mismatch = MISMATCH_DETECTED;
		}

		/* check CACHE LOCK bit in NS Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GNSR_ACR);
		if (0U == (val & SMMU_ACR_CACHE_LOCK_ENABLE_BIT)) {
			ERROR("%s: Mismatch - smmu_id=%d, GNSR_ACR=%x\n",
				__func__, smmu_id, val);
			mismatch = MISMATCH_DETECTED;
		}

		/* verify TCU prefetch for all contexts is disabled */
		ctx_base = (SMMU_GSR0_PGSIZE_64K * SMMU_NUM_CONTEXTS) +
			SMMU_CBn_ACTLR;
		for (cb_idx = 0U; cb_idx < SMMU_CONTEXT_BANK_MAX_IDX; cb_idx++) {
			val = tegra_smmu_read_32(smmu_id,
				ctx_base + (SMMU_GSR0_PGSIZE_64K * cb_idx));
			if (0U != (val & SMMU_CBn_ACTLR_CPRE_BIT)) {
				ERROR("%s: Mismatch - smmu_id=%d, cb_idx=%d, GSR0_PGSIZE_64K=%x\n",
					__func__, smmu_id, cb_idx, val);
				mismatch = MISMATCH_DETECTED;
			}
		}
	}

	/* Treat configuration mismatch as fatal */
	if ((mismatch == MISMATCH_DETECTED) && tegra_platform_is_silicon()) {
		panic();
	}
}
