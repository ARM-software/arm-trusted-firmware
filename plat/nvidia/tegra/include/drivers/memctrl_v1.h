/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMCTRL_V1_H
#define MEMCTRL_V1_H

#include <lib/mmio.h>

#include <tegra_def.h>

/* SMMU registers */
#define MC_SMMU_CONFIG_0			0x10U
#define  MC_SMMU_CONFIG_0_SMMU_ENABLE_DISABLE	0U
#define  MC_SMMU_CONFIG_0_SMMU_ENABLE_ENABLE	1U
#define MC_SMMU_TLB_CONFIG_0			0x14U
#define  MC_SMMU_TLB_CONFIG_0_RESET_VAL		0x20000010U
#define MC_SMMU_PTC_CONFIG_0			0x18U
#define  MC_SMMU_PTC_CONFIG_0_RESET_VAL		0x2000003fU
#define MC_SMMU_TLB_FLUSH_0			0x30U
#define  TLB_FLUSH_VA_MATCH_ALL			0U
#define  TLB_FLUSH_ASID_MATCH_DISABLE		0U
#define  TLB_FLUSH_ASID_MATCH_SHIFT		31U
#define  MC_SMMU_TLB_FLUSH_ALL		\
	 (TLB_FLUSH_VA_MATCH_ALL | 	\
	 (TLB_FLUSH_ASID_MATCH_DISABLE << TLB_FLUSH_ASID_MATCH_SHIFT))
#define MC_SMMU_PTC_FLUSH_0			0x34U
#define  MC_SMMU_PTC_FLUSH_ALL			0U
#define MC_SMMU_ASID_SECURITY_0			0x38U
#define  MC_SMMU_ASID_SECURITY			0U
#define MC_SMMU_TRANSLATION_ENABLE_0_0		0x228U
#define MC_SMMU_TRANSLATION_ENABLE_1_0		0x22cU
#define MC_SMMU_TRANSLATION_ENABLE_2_0		0x230U
#define MC_SMMU_TRANSLATION_ENABLE_3_0		0x234U
#define MC_SMMU_TRANSLATION_ENABLE_4_0		0xb98U
#define  MC_SMMU_TRANSLATION_ENABLE		(~0)

/* MC IRAM aperture registers */
#define MC_IRAM_BASE_LO				0x65CU
#define MC_IRAM_TOP_LO				0x660U
#define MC_IRAM_BASE_TOP_HI			0x980U
#define MC_IRAM_REG_CTRL			0x964U
#define  MC_DISABLE_IRAM_CFG_WRITES		1U

static inline uint32_t tegra_mc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_BASE + off);
}

static inline void tegra_mc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_BASE + off, val);
}

#endif /* MEMCTRL_V1_H */
