/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMCTRL_V2_H
#define MEMCTRL_V2_H

#include <arch.h>

#include <tegra_def.h>

/*******************************************************************************
 * Memory Controller SMMU Bypass config register
 ******************************************************************************/
#define MC_SMMU_BYPASS_CONFIG			0x1820U
#define MC_SMMU_BYPASS_CTRL_MASK		0x3U
#define MC_SMMU_BYPASS_CTRL_SHIFT		0U
#define MC_SMMU_CTRL_TBU_BYPASS_ALL		(0U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_RSVD			(1U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID	(2U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_NONE		(3U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT	(1U << 31)
#define MC_SMMU_BYPASS_CONFIG_SETTINGS		(MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT | \
						 MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID)

#ifndef	__ASSEMBLER__

#include <assert.h>

typedef struct mc_regs {
	uint32_t reg;
	uint32_t val;
} mc_regs_t;

#define mc_smmu_bypass_cfg \
	{ \
		.reg = TEGRA_MC_BASE + MC_SMMU_BYPASS_CONFIG, \
		.val = 0x00000000U, \
	}

#define _START_OF_TABLE_ \
	{ \
		.reg = 0xCAFE05C7U, \
		.val = 0x00000000U, \
	}

#define _END_OF_TABLE_ \
	{ \
		.reg = 0xFFFFFFFFU, \
		.val = 0xFFFFFFFFU, \
	}

#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__

#include <lib/mmio.h>

static inline uint32_t tegra_mc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_BASE + off);
}

static inline void tegra_mc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_BASE + off, val);
}

#if defined(TEGRA_MC_STREAMID_BASE)
static inline uint32_t tegra_mc_streamid_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_STREAMID_BASE + off);
}

static inline void tegra_mc_streamid_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_STREAMID_BASE + off, val);
	assert(mmio_read_32(TEGRA_MC_STREAMID_BASE + off) == val);
}
#endif

void plat_memctrl_setup(void);

void plat_memctrl_restore(void);
mc_regs_t *plat_memctrl_get_sys_suspend_ctx(void);

/*******************************************************************************
 * Handler to save MC settings before "System Suspend" to TZDRAM
 *
 * Implemented by Tegra common memctrl_v2 driver under common/drivers/memctrl
 ******************************************************************************/
void tegra_mc_save_context(uint64_t mc_ctx_addr);

/*******************************************************************************
 * Handler to program the scratch registers with TZDRAM settings for the
 * resume firmware.
 *
 * Implemented by SoCs under tegra/soc/txxx
 ******************************************************************************/
void plat_memctrl_tzdram_setup(uint64_t phys_base, uint64_t size_in_bytes);

#endif /* __ASSEMBLER__ */

#endif /* MEMCTRL_V2_H */
