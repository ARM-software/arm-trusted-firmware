/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <memctrl.h>
#include <memctrl_v1.h>
#include <tegra_def.h>

/* Video Memory base and size (live values) */
static uint64_t video_mem_base;
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
	INFO("Tegra Memory Controller (v1)\n");

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
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
			  (uint32_t)(video_mem_base >> 32));
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO, (uint32_t)video_mem_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, video_mem_size);
}

/*
 * Restore Memory Controller settings after "System Suspend"
 */
void tegra_memctrl_restore_settings(void)
{
	tegra_memctrl_setup();
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
	int ret;

	/*
	 * Map the NS memory first, clean it and then unmap it.
	 */
	ret = mmap_add_dynamic_region(non_overlap_area_start, /* PA */
				non_overlap_area_start, /* VA */
				non_overlap_area_size, /* size */
				MT_NS | MT_RW | MT_EXECUTE_NEVER |
				MT_NON_CACHEABLE); /* attrs */
	assert(ret == 0);

	zeromem((void *)non_overlap_area_start, non_overlap_area_size);
	flush_dcache_range(non_overlap_area_start, non_overlap_area_size);

	mmap_remove_dynamic_region(non_overlap_area_start,
		non_overlap_area_size);
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
	unsigned long long non_overlap_area_size;

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

done:
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI, (uint32_t)(phys_base >> 32));
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, size_in_bytes >> 20);

	/* store new values */
	video_mem_base = phys_base;
	video_mem_size = size_in_bytes >> 20;
}

/*
 * During boot, USB3 and flash media (SDMMC/SATA) devices need access to
 * IRAM. Because these clients connect to the MC and do not have a direct
 * path to the IRAM, the MC implements AHB redirection during boot to allow
 * path to IRAM. In this mode, accesses to a programmed memory address aperture
 * are directed to the AHB bus, allowing access to the IRAM. The AHB aperture
 * is defined by the IRAM_BASE_LO and IRAM_BASE_HI registers, which are
 * initialized to disable this aperture.
 *
 * Once bootup is complete, we must program IRAM base to 0xffffffff and
 * IRAM top to 0x00000000, thus disabling access to IRAM. DRAM is then
 * potentially accessible in this address range. These aperture registers
 * also have an access_control/lock bit. After disabling the aperture, the
 * access_control register should be programmed to lock the registers.
 */
void tegra_memctrl_disable_ahb_redirection(void)
{
	/* program the aperture registers */
	tegra_mc_write_32(MC_IRAM_BASE_LO, 0xFFFFFFFF);
	tegra_mc_write_32(MC_IRAM_TOP_LO, 0);
	tegra_mc_write_32(MC_IRAM_BASE_TOP_HI, 0);

	/* lock the aperture registers */
	tegra_mc_write_32(MC_IRAM_REG_CTRL, MC_DISABLE_IRAM_CFG_WRITES);
}

void tegra_memctrl_clear_pending_interrupts(void)
{
	uint32_t mcerr;

	/* check if there are any pending interrupts */
	mcerr = mmio_read_32(TEGRA_MC_BASE + MC_INTSTATUS);

	if (mcerr != (uint32_t)0U) { /* should not see error here */
		WARN("MC_INTSTATUS = 0x%x (should be zero)\n", mcerr);
		mmio_write_32((TEGRA_MC_BASE + MC_INTSTATUS),  mcerr);
	}
}
