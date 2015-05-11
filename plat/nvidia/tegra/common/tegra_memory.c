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

#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <tegra_def.h>

#define MC_SMMU_CONFIG_0			0x10
#define MC_SMMU_CONFIG_0_SMMU_ENABLE_DISABLE	0
#define MC_SMMU_CONFIG_0_SMMU_ENABLE_ENABLE	1

#define MC_SMMU_TLB_CONFIG_0			0x14
#define MC_SMMU_TLB_CONFIG_0_RESET_VAL		0x20000010

#define MC_SMMU_PTC_CONFIG_0			0x18
#define MC_SMMU_PTC_CONFIG_0_RESET_VAL		0x2000003f

#define MC_SMMU_TLB_FLUSH_0			0x30
#define TLB_FLUSH_VA_MATCH_ALL		0
#define TLB_FLUSH_ASID_MATCH_DISABLE	0
#define TLB_FLUSH_ASID_MATCH_SHIFT	31
#define MC_SMMU_TLB_FLUSH_ALL			\
	(TLB_FLUSH_VA_MATCH_ALL | 		\
	(TLB_FLUSH_ASID_MATCH_DISABLE << TLB_FLUSH_ASID_MATCH_SHIFT))

#define MC_SMMU_PTC_FLUSH_0			0x34
#define MC_SMMU_PTC_FLUSH_ALL			0

#define MC_SMMU_ASID_SECURITY_0			0x38
#define ASID_SECURITY		(0)

#define MC_SMMU_TRANSLATION_ENABLE_0_0		0x228
#define MC_SMMU_TRANSLATION_ENABLE_1_0		0x22c
#define MC_SMMU_TRANSLATION_ENABLE_2_0		0x230
#define MC_SMMU_TRANSLATION_ENABLE_3_0		0x234
#define MC_SMMU_TRANSLATION_ENABLE_4_0		0xb98

#define TRANSLATION_ENABLE	(~0)

/*
 * Init SMMU.
 */
void tegra_memory_setup(void)
{
	uintptr_t mc_base = TEGRA_MC_BASE;

	/*
	 * Setup the Memory controller to allow only secure accesses to
	 * the TZDRAM carveout
	 */
	INFO("Configuring SMMU\n");

	/* allow translations for all MC engines */
	mmio_write_32(mc_base + MC_SMMU_TRANSLATION_ENABLE_0_0,
			(unsigned int)TRANSLATION_ENABLE);
	mmio_write_32(mc_base + MC_SMMU_TRANSLATION_ENABLE_1_0,
			(unsigned int)TRANSLATION_ENABLE);
	mmio_write_32(mc_base + MC_SMMU_TRANSLATION_ENABLE_2_0,
			(unsigned int)TRANSLATION_ENABLE);
	mmio_write_32(mc_base + MC_SMMU_TRANSLATION_ENABLE_3_0,
			(unsigned int)TRANSLATION_ENABLE);
	mmio_write_32(mc_base + MC_SMMU_TRANSLATION_ENABLE_4_0,
			(unsigned int)TRANSLATION_ENABLE);

	mmio_write_32(mc_base + MC_SMMU_ASID_SECURITY_0, ASID_SECURITY);

	mmio_write_32(mc_base + MC_SMMU_TLB_CONFIG_0,
			MC_SMMU_TLB_CONFIG_0_RESET_VAL);
	mmio_write_32(mc_base + MC_SMMU_PTC_CONFIG_0,
			MC_SMMU_PTC_CONFIG_0_RESET_VAL);

	/* flush PTC and TLB */
	mmio_write_32(mc_base + MC_SMMU_PTC_FLUSH_0, MC_SMMU_PTC_FLUSH_ALL);
	(void) mmio_read_32(mc_base + MC_SMMU_CONFIG_0);	/* read to flush writes */
	mmio_write_32(mc_base + MC_SMMU_TLB_FLUSH_0, MC_SMMU_TLB_FLUSH_ALL);

	/* enable SMMU */
	mmio_write_32(mc_base + MC_SMMU_CONFIG_0, MC_SMMU_CONFIG_0_SMMU_ENABLE_ENABLE);
	(void) mmio_read_32(mc_base + MC_SMMU_CONFIG_0);	/* read to flush writes */
}
