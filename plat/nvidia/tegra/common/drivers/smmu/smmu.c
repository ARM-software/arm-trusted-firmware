/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
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

/*
 * Save SMMU settings before "System Suspend" to TZDRAM
 */
void tegra_smmu_save_context(uint64_t smmu_ctx_addr)
{
	uint32_t i, num_entries = 0;
	smmu_regs_t *smmu_ctx_regs;
	const plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t tzdram_base = params_from_bl2->tzdram_base;
	uint64_t tzdram_end = tzdram_base + params_from_bl2->tzdram_size;
	uint32_t reg_id1, pgshift, cb_size;

	/* sanity check SMMU settings c*/
	reg_id1 = mmio_read_32((TEGRA_SMMU0_BASE + SMMU_GNSR0_IDR1));
	pgshift = ((reg_id1 & ID1_PAGESIZE) != 0U) ? 16U : 12U;
	cb_size = ((uint32_t)2 << pgshift) * \
	((uint32_t)1 << (((reg_id1 >> ID1_NUMPAGENDXB_SHIFT) & ID1_NUMPAGENDXB_MASK) + 1U));

	assert(!((pgshift != PGSHIFT) || (cb_size != CB_SIZE)));
	assert((smmu_ctx_addr >= tzdram_base) && (smmu_ctx_addr <= tzdram_end));

	/* get SMMU context table */
	smmu_ctx_regs = plat_get_smmu_ctx();
	assert(smmu_ctx_regs != NULL);

	/*
	 * smmu_ctx_regs[0].val contains the size of the context table minus
	 * the last entry. Sanity check the table size before we start with
	 * the context save operation.
	 */
	while ((smmu_ctx_regs[num_entries].reg != 0xFFFFFFFFU)) {
		num_entries++;
	}

	/* panic if the sizes do not match */
	if (num_entries != smmu_ctx_regs[0].val) {
		ERROR("SMMU context size mismatch!");
		panic();
	}

	/* save SMMU register values */
	for (i = 1U; i < num_entries; i++) {
		smmu_ctx_regs[i].val = mmio_read_32(smmu_ctx_regs[i].reg);
	}

	/* increment by 1 to take care of the last entry */
	num_entries++;

	/* Save SMMU config settings */
	(void)memcpy16((uint8_t *)smmu_ctx_addr, (uint8_t *)smmu_ctx_regs,
			(sizeof(smmu_regs_t) * num_entries));

	/* save the SMMU table address */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SMMU_TABLE_ADDR_LO,
		(uint32_t)smmu_ctx_addr);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_SMMU_TABLE_ADDR_HI,
		(uint32_t)(smmu_ctx_addr >> 32));
}

#define SMMU_NUM_CONTEXTS		64
#define SMMU_CONTEXT_BANK_MAX_IDX	64

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
