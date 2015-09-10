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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <memctrl.h>
#include <string.h>
#include <tegra_def.h>
#include <xlat_tables.h>

extern void zeromem16(void *mem, unsigned int length);

#define TEGRA_GPU_RESET_REG_OFFSET	0x28c
#define  GPU_RESET_BIT			(1 << 24)

/* Video Memory base and size (live values) */
static uintptr_t video_mem_base;
static uint64_t video_mem_size;

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

	/* video memory carveout */
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE, video_mem_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, video_mem_size);
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

static void tegra_clear_videomem(uintptr_t non_overlap_area_start,
				 unsigned long long non_overlap_area_size)
{
	/*
	 * Perform cache maintenance to ensure that the non-overlapping area is
	 * zeroed out. The first invalidation of this range ensures that
	 * possible evictions of dirty cache lines do not interfere with the
	 * 'zeromem16' operation. Other CPUs could speculatively prefetch the
	 * main memory contents of this area between the first invalidation and
	 * the 'zeromem16' operation. The second invalidation ensures that any
	 * such cache lines are removed as well.
	 */
	inv_dcache_range(non_overlap_area_start, non_overlap_area_size);
	zeromem16((void *)non_overlap_area_start, non_overlap_area_size);
	inv_dcache_range(non_overlap_area_start, non_overlap_area_size);
}

/*
 * Program the Video Memory carveout region
 *
 * phys_base = physical base of aperture
 * size_in_bytes = size of aperture in bytes
 */
void tegra_memctrl_videomem_setup(uint64_t phys_base, uint32_t size_in_bytes)
{
	uintptr_t vmem_end_old = video_mem_base + (video_mem_size << 20);
	uintptr_t vmem_end_new = phys_base + size_in_bytes;
	uint32_t regval;
	unsigned long long non_overlap_area_size;

	/*
	 * The GPU is the user of the Video Memory region. In order to
	 * transition to the new memory region smoothly, we program the
	 * new base/size ONLY if the GPU is in reset mode.
	 */
	regval = mmio_read_32(TEGRA_CAR_RESET_BASE + TEGRA_GPU_RESET_REG_OFFSET);
	if ((regval & GPU_RESET_BIT) == 0) {
		ERROR("GPU not in reset! Video Memory setup failed\n");
		return;
	}

	/*
	 * Setup the Memory controller to restrict CPU accesses to the Video
	 * Memory region
	 */
	INFO("Configuring Video Memory Carveout\n");

	/*
	 * Configure Memory Controller directly for the first time.
	 */
	if (video_mem_base == 0)
		goto done;

	/*
	 * Clear the old regions now being exposed. The following cases
	 * can occur -
	 *
	 * 1. clear whole old region (no overlap with new region)
	 * 2. clear old sub-region below new base
	 * 3. clear old sub-region above new end
	 */
	INFO("Cleaning previous Video Memory Carveout\n");

	disable_mmu_el3();
	if (phys_base > vmem_end_old || video_mem_base > vmem_end_new) {
		tegra_clear_videomem(video_mem_base, video_mem_size << 20);
	} else {
		if (video_mem_base < phys_base) {
			non_overlap_area_size = phys_base - video_mem_base;
			tegra_clear_videomem(video_mem_base, non_overlap_area_size);
		}
		if (vmem_end_old > vmem_end_new) {
			non_overlap_area_size = vmem_end_old - vmem_end_new;
			tegra_clear_videomem(vmem_end_new, non_overlap_area_size);
		}
	}
	enable_mmu_el3(0);

done:
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE, phys_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, size_in_bytes >> 20);

	/* store new values */
	video_mem_base = phys_base;
	video_mem_size = size_in_bytes >> 20;
}
