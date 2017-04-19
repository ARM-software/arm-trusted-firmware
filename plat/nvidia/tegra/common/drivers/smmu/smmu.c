/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <platform_def.h>
#include <smmu.h>
#include <string.h>
#include <tegra_private.h>

/* SMMU IDs currently supported by the driver */
enum {
	TEGRA_SMMU0,
	TEGRA_SMMU1,
	TEGRA_SMMU2
};

static uint32_t tegra_smmu_read_32(uint32_t smmu_id, uint32_t off)
{
#if defined(TEGRA_SMMU0_BASE)
	if (smmu_id == TEGRA_SMMU0)
		return mmio_read_32(TEGRA_SMMU0_BASE + off);
#endif

#if defined(TEGRA_SMMU1_BASE)
	if (smmu_id == TEGRA_SMMU1)
		return mmio_read_32(TEGRA_SMMU1_BASE + off);
#endif

#if defined(TEGRA_SMMU2_BASE)
	if (smmu_id == TEGRA_SMMU2)
		return mmio_read_32(TEGRA_SMMU2_BASE + off);
#endif

	return 0;
}

static void tegra_smmu_write_32(uint32_t smmu_id,
			uint32_t off, uint32_t val)
{
#if defined(TEGRA_SMMU0_BASE)
	if (smmu_id == TEGRA_SMMU0)
		mmio_write_32(TEGRA_SMMU0_BASE + off, val);
#endif

#if defined(TEGRA_SMMU1_BASE)
	if (smmu_id == TEGRA_SMMU1)
		mmio_write_32(TEGRA_SMMU1_BASE + off, val);
#endif

#if defined(TEGRA_SMMU2_BASE)
	if (smmu_id == TEGRA_SMMU2)
		mmio_write_32(TEGRA_SMMU2_BASE + off, val);
#endif
}

/*
 * Save SMMU settings before "System Suspend" to TZDRAM
 */
void tegra_smmu_save_context(uint64_t smmu_ctx_addr)
{
	uint32_t i;
	smmu_regs_t *smmu_ctx_regs;
#if DEBUG
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t tzdram_base = params_from_bl2->tzdram_base;
	uint64_t tzdram_end = tzdram_base + params_from_bl2->tzdram_size;
	uint32_t reg_id1, pgshift, cb_size;

	/* sanity check SMMU settings c*/
	reg_id1 = mmio_read_32((TEGRA_SMMU0_BASE + SMMU_GNSR0_IDR1));
	pgshift = (reg_id1 & ID1_PAGESIZE) ? 16 : 12;
	cb_size = (2 << pgshift) * \
	(1 << (((reg_id1 >> ID1_NUMPAGENDXB_SHIFT) & ID1_NUMPAGENDXB_MASK) + 1));

	assert(!((pgshift != PGSHIFT) || (cb_size != CB_SIZE)));
#endif

	assert((smmu_ctx_addr >= tzdram_base) && (smmu_ctx_addr <= tzdram_end));

	/* get SMMU context table */
	smmu_ctx_regs = plat_get_smmu_ctx();
	assert(smmu_ctx_regs);

	/* save SMMU register values */
	for (i = 1; i < smmu_ctx_regs[0].val; i++)
		smmu_ctx_regs[i].val = mmio_read_32(smmu_ctx_regs[i].reg);

	/* Save SMMU config settings */
	memcpy16((void *)(uintptr_t)smmu_ctx_addr, (void *)smmu_ctx_regs,
		 sizeof(smmu_regs_t));

	/* save the SMMU table address */
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV11_LO,
		(uint32_t)smmu_ctx_addr);
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV11_HI,
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

	for (smmu_id = 0; smmu_id < NUM_SMMU_DEVICES; smmu_id++) {
		/* Program the SMMU pagesize and reset CACHE_LOCK bit */
		val = tegra_smmu_read_32(smmu_id, SMMU_GSR0_SECURE_ACR);
		val |= SMMU_GSR0_PGSIZE_64K;
		val &= ~SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GSR0_SECURE_ACR, val);

		/* reset CACHE LOCK bit for NS Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GNSR_ACR);
		val &= ~SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GNSR_ACR, val);

		/* disable TCU prefetch for all contexts */
		ctx_base = (SMMU_GSR0_PGSIZE_64K * SMMU_NUM_CONTEXTS)
				+ SMMU_CBn_ACTLR;
		for (cb_idx = 0; cb_idx < SMMU_CONTEXT_BANK_MAX_IDX; cb_idx++) {
			val = tegra_smmu_read_32(smmu_id,
				ctx_base + (SMMU_GSR0_PGSIZE_64K * cb_idx));
			val &= ~SMMU_CBn_ACTLR_CPRE_BIT;
			tegra_smmu_write_32(smmu_id, ctx_base +
				(SMMU_GSR0_PGSIZE_64K * cb_idx), val);
		}

		/* set CACHE LOCK bit for NS Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GNSR_ACR);
		val |= SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GNSR_ACR, val);

		/* set CACHE LOCK bit for S Aux. Config. Register */
		val = tegra_smmu_read_32(smmu_id, SMMU_GSR0_SECURE_ACR);
		val |= SMMU_ACR_CACHE_LOCK_ENABLE_BIT;
		tegra_smmu_write_32(smmu_id, SMMU_GSR0_SECURE_ACR, val);
	}
}
