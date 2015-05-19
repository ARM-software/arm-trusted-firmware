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
#include <memctrl.h>
#include <tegra_def.h>

/*
 * Init SMMU.
 */
void tegra_memctrl_setup(void)
{
	/*
	 * Setup the Memory controller to allow only secure accesses to
	 * the TZDRAM carveout
	 */
	INFO("Configuring SMMU\n");

	/* allow translations for all MC engines */
	tegra_mc_write_32(MC_SMMU_TRANSLATION_ENABLE_0_0,
			(unsigned int)MC_SMMU_TRANSLATION_ENABLE);
	tegra_mc_write_32(MC_SMMU_TRANSLATION_ENABLE_1_0,
			(unsigned int)MC_SMMU_TRANSLATION_ENABLE);
	tegra_mc_write_32(MC_SMMU_TRANSLATION_ENABLE_2_0,
			(unsigned int)MC_SMMU_TRANSLATION_ENABLE);
	tegra_mc_write_32(MC_SMMU_TRANSLATION_ENABLE_3_0,
			(unsigned int)MC_SMMU_TRANSLATION_ENABLE);
	tegra_mc_write_32(MC_SMMU_TRANSLATION_ENABLE_4_0,
			(unsigned int)MC_SMMU_TRANSLATION_ENABLE);

	tegra_mc_write_32(MC_SMMU_ASID_SECURITY_0, MC_SMMU_ASID_SECURITY);

	tegra_mc_write_32(MC_SMMU_TLB_CONFIG_0, MC_SMMU_TLB_CONFIG_0_RESET_VAL);
	tegra_mc_write_32(MC_SMMU_PTC_CONFIG_0, MC_SMMU_PTC_CONFIG_0_RESET_VAL);

	/* flush PTC and TLB */
	tegra_mc_write_32(MC_SMMU_PTC_FLUSH_0, MC_SMMU_PTC_FLUSH_ALL);
	(void)tegra_mc_read_32(MC_SMMU_CONFIG_0); /* read to flush writes */
	tegra_mc_write_32(MC_SMMU_TLB_FLUSH_0, MC_SMMU_TLB_FLUSH_ALL);

	/* enable SMMU */
	tegra_mc_write_32(MC_SMMU_CONFIG_0,
			  MC_SMMU_CONFIG_0_SMMU_ENABLE_ENABLE);
	(void)tegra_mc_read_32(MC_SMMU_CONFIG_0); /* read to flush writes */
}

/*
 * Secure the BL31 DRAM aperture.
 *
 * phys_base = physical base of TZDRAM aperture
 * size_in_bytes = size of aperture in bytes
 */
void tegra_memctrl_tzdram_setup(uint64_t phys_base, uint32_t size_in_bytes)
{
	/*
	 * Setup the Memory controller to allow only secure accesses to
	 * the TZDRAM carveout
	 */
	INFO("Configuring TrustZone DRAM Memory Carveout\n");

	tegra_mc_write_32(MC_SECURITY_CFG0_0, phys_base);
	tegra_mc_write_32(MC_SECURITY_CFG1_0, size_in_bytes >> 20);
}
