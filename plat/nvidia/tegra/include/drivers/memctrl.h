/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __MEMCTRL_H__
#define __MEMCTRL_H__

#include <mmio.h>
#include <tegra_def.h>

/* SMMU registers */
#define MC_SMMU_CONFIG_0			0x10
#define  MC_SMMU_CONFIG_0_SMMU_ENABLE_DISABLE	0
#define  MC_SMMU_CONFIG_0_SMMU_ENABLE_ENABLE	1
#define MC_SMMU_TLB_CONFIG_0			0x14
#define  MC_SMMU_TLB_CONFIG_0_RESET_VAL		0x20000010
#define MC_SMMU_PTC_CONFIG_0			0x18
#define  MC_SMMU_PTC_CONFIG_0_RESET_VAL		0x2000003f
#define MC_SMMU_TLB_FLUSH_0			0x30
#define  TLB_FLUSH_VA_MATCH_ALL			0
#define  TLB_FLUSH_ASID_MATCH_DISABLE		0
#define  TLB_FLUSH_ASID_MATCH_SHIFT		31
#define  MC_SMMU_TLB_FLUSH_ALL		\
	 (TLB_FLUSH_VA_MATCH_ALL | 	\
	 (TLB_FLUSH_ASID_MATCH_DISABLE << TLB_FLUSH_ASID_MATCH_SHIFT))
#define MC_SMMU_PTC_FLUSH_0			0x34
#define  MC_SMMU_PTC_FLUSH_ALL			0
#define MC_SMMU_ASID_SECURITY_0			0x38
#define  MC_SMMU_ASID_SECURITY			0
#define MC_SMMU_TRANSLATION_ENABLE_0_0		0x228
#define MC_SMMU_TRANSLATION_ENABLE_1_0		0x22c
#define MC_SMMU_TRANSLATION_ENABLE_2_0		0x230
#define MC_SMMU_TRANSLATION_ENABLE_3_0		0x234
#define MC_SMMU_TRANSLATION_ENABLE_4_0		0xb98
#define  MC_SMMU_TRANSLATION_ENABLE		(~0)

/* TZDRAM carveout configuration registers */
#define MC_SECURITY_CFG0_0			0x70
#define MC_SECURITY_CFG1_0			0x74

/* Video Memory carveout configuration registers */
#define MC_VIDEO_PROTECT_BASE			0x648
#define MC_VIDEO_PROTECT_SIZE_MB		0x64c

static inline uint32_t tegra_mc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_BASE + off);
}

static inline void tegra_mc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_BASE + off, val);
}

void tegra_memctrl_setup(void);
void tegra_memctrl_tzdram_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_videomem_setup(uint64_t phys_base, uint32_t size_in_bytes);

#endif /* __MEMCTRL_H__ */
