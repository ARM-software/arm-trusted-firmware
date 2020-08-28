/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <mce.h>
#include <memctrl.h>
#include <memctrl_v2.h>
#include <smmu.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/* Video Memory base and size (live values) */
static uint64_t video_mem_base;
static uint64_t video_mem_size_mb;

/*
 * Init Memory controller during boot.
 */
void tegra_memctrl_setup(void)
{
	INFO("Tegra Memory Controller (v2)\n");

	/* Initialize the System memory management unit */
	tegra_smmu_init();

	/* allow platforms to program custom memory controller settings */
	plat_memctrl_setup();

	/*
	 * All requests at boot time, and certain requests during
	 * normal run time, are physically addressed and must bypass
	 * the SMMU. The client hub logic implements a hardware bypass
	 * path around the Translation Buffer Units (TBU). During
	 * boot-time, the SMMU_BYPASS_CTRL register (which defaults to
	 * TBU_BYPASS mode) will be used to steer all requests around
	 * the uninitialized TBUs. During normal operation, this register
	 * is locked into TBU_BYPASS_SID config, which routes requests
	 * with special StreamID 0x7f on the bypass path and all others
	 * through the selected TBU. This is done to disable SMMU Bypass
	 * mode, as it could be used to circumvent SMMU security checks.
	 */
	tegra_mc_write_32(MC_SMMU_BYPASS_CONFIG,
			  MC_SMMU_BYPASS_CONFIG_SETTINGS);
}

/*
 * Restore Memory Controller settings after "System Suspend"
 */
void tegra_memctrl_restore_settings(void)
{
	/* restore platform's memory controller settings */
	plat_memctrl_restore();

	/* video memory carveout region */
	if (video_mem_base != 0ULL) {
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO,
				  (uint32_t)video_mem_base);
		assert(tegra_mc_read_32(MC_VIDEO_PROTECT_BASE_LO)
			 == (uint32_t)video_mem_base);
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
				  (uint32_t)(video_mem_base >> 32));
		assert(tegra_mc_read_32(MC_VIDEO_PROTECT_BASE_HI)
			 == (uint32_t)(video_mem_base >> 32));
		tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB,
				  (uint32_t)video_mem_size_mb);
		assert(tegra_mc_read_32(MC_VIDEO_PROTECT_SIZE_MB)
			 == (uint32_t)video_mem_size_mb);

		/*
		 * MCE propagates the VideoMem configuration values across the
		 * CCPLEX.
		 */
		mce_update_gsc_videomem();
	}
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
	 * Perform platform specific steps.
	 */
	plat_memctrl_tzdram_setup(phys_base, size_in_bytes);
}

/*
 * Secure the BL31 TZRAM aperture.
 *
 * phys_base = physical base of TZRAM aperture
 * size_in_bytes = size of aperture in bytes
 */
void tegra_memctrl_tzram_setup(uint64_t phys_base, uint32_t size_in_bytes)
{
	; /* do nothing */
}

/*
 * Save MC settings before "System Suspend" to TZDRAM
 */
void tegra_mc_save_context(uint64_t mc_ctx_addr)
{
	uint32_t i, num_entries = 0;
	mc_regs_t *mc_ctx_regs;
	const plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();
	uint64_t tzdram_base = params_from_bl2->tzdram_base;
	uint64_t tzdram_end = tzdram_base + params_from_bl2->tzdram_size;

	assert((mc_ctx_addr >= tzdram_base) && (mc_ctx_addr <= tzdram_end));

	/* get MC context table */
	mc_ctx_regs = plat_memctrl_get_sys_suspend_ctx();
	assert(mc_ctx_regs != NULL);

	/*
	 * mc_ctx_regs[0].val contains the size of the context table minus
	 * the last entry. Sanity check the table size before we start with
	 * the context save operation.
	 */
	while (mc_ctx_regs[num_entries].reg != 0xFFFFFFFFU) {
		num_entries++;
	}

	/* panic if the sizes do not match */
	if (num_entries != mc_ctx_regs[0].val) {
		ERROR("MC context size mismatch!");
		panic();
	}

	/* save MC register values */
	for (i = 1U; i < num_entries; i++) {
		mc_ctx_regs[i].val = mmio_read_32(mc_ctx_regs[i].reg);
	}

	/* increment by 1 to take care of the last entry */
	num_entries++;

	/* Save MC config settings */
	(void)memcpy((void *)mc_ctx_addr, mc_ctx_regs,
			sizeof(mc_regs_t) * num_entries);

	/* save the MC table address */
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_MC_TABLE_ADDR_LO,
		(uint32_t)mc_ctx_addr);
	assert(mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_MC_TABLE_ADDR_LO)
		== (uint32_t)mc_ctx_addr);
	mmio_write_32(TEGRA_SCRATCH_BASE + SCRATCH_MC_TABLE_ADDR_HI,
		(uint32_t)(mc_ctx_addr >> 32));
	assert(mmio_read_32(TEGRA_SCRATCH_BASE + SCRATCH_MC_TABLE_ADDR_HI)
		== (uint32_t)(mc_ctx_addr >> 32));
}

static void tegra_lock_videomem_nonoverlap(uint64_t phys_base,
					   uint64_t size_in_bytes)
{
	uint32_t index;
	uint64_t total_128kb_blocks = size_in_bytes >> 17;
	uint64_t residual_4kb_blocks = (size_in_bytes & (uint32_t)0x1FFFF) >> 12;
	uint64_t val;

	/*
	 * Reset the access configuration registers to restrict access to
	 * old Videomem aperture
	 */
	for (index = MC_VIDEO_PROTECT_CLEAR_ACCESS_CFG0;
	     index < ((uint32_t)MC_VIDEO_PROTECT_CLEAR_ACCESS_CFG0 + (uint32_t)MC_GSC_CONFIG_REGS_SIZE);
	     index += 4U) {
		tegra_mc_write_32(index, 0);
	}

	/*
	 * Set the base. It must be 4k aligned, at least.
	 */
	assert((phys_base & (uint64_t)0xFFF) == 0U);
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_BASE_HI,
		(uint32_t)(phys_base >> 32) & (uint32_t)MC_GSC_BASE_HI_MASK);

	/*
	 * Set the aperture size
	 *
	 * total size = (number of 128KB blocks) + (number of remaining 4KB
	 * blocks)
	 *
	 */
	val = (uint32_t)((residual_4kb_blocks << MC_GSC_SIZE_RANGE_4KB_SHIFT) |
			 total_128kb_blocks);
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_SIZE, (uint32_t)val);

	/*
	 * Lock the configuration settings by enabling TZ-only lock and
	 * locking the configuration against any future changes from NS
	 * world.
	 */
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_CFG,
			  (uint32_t)MC_GSC_ENABLE_TZ_LOCK_BIT);

	/*
	 * MCE propagates the GSC configuration values across the
	 * CCPLEX.
	 */
}

static void tegra_unlock_videomem_nonoverlap(void)
{
	/* Clear the base */
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_BASE_LO, 0);
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_BASE_HI, 0);

	/* Clear the size */
	tegra_mc_write_32(MC_VIDEO_PROTECT_CLEAR_SIZE, 0);
}

static void tegra_clear_videomem(uintptr_t non_overlap_area_start,
				 unsigned long long non_overlap_area_size)
{
	int ret;

	INFO("Cleaning previous Video Memory Carveout\n");

	/*
	 * Map the NS memory first, clean it and then unmap it.
	 */
	ret = mmap_add_dynamic_region(non_overlap_area_start, /* PA */
				non_overlap_area_start, /* VA */
				non_overlap_area_size, /* size */
				MT_DEVICE | MT_RW | MT_NS); /* attrs */
	assert(ret == 0);

	zeromem((void *)non_overlap_area_start, non_overlap_area_size);
	flush_dcache_range(non_overlap_area_start, non_overlap_area_size);

	ret = mmap_remove_dynamic_region(non_overlap_area_start,
		non_overlap_area_size);
	assert(ret == 0);
}

static void tegra_clear_videomem_nonoverlap(uintptr_t phys_base,
		unsigned long size_in_bytes)
{
	uintptr_t vmem_end_old = video_mem_base + (video_mem_size_mb << 20);
	uintptr_t vmem_end_new = phys_base + size_in_bytes;
	unsigned long long non_overlap_area_size;

	/*
	 * Clear the old regions now being exposed. The following cases
	 * can occur -
	 *
	 * 1. clear whole old region (no overlap with new region)
	 * 2. clear old sub-region below new base
	 * 3. clear old sub-region above new end
	 */
	if ((phys_base > vmem_end_old) || (video_mem_base > vmem_end_new)) {
		tegra_clear_videomem(video_mem_base,
				     video_mem_size_mb << 20U);
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
}

/*
 * Program the Video Memory carveout region
 *
 * phys_base = physical base of aperture
 * size_in_bytes = size of aperture in bytes
 */
void tegra_memctrl_videomem_setup(uint64_t phys_base, uint32_t size_in_bytes)
{
	/*
	 * Setup the Memory controller to restrict CPU accesses to the Video
	 * Memory region
	 */

	INFO("Configuring Video Memory Carveout\n");

	if (video_mem_base != 0U) {
		/*
		 * Lock the non overlapping memory being cleared so that
		 * other masters do not accidently write to it. The memory
		 * would be unlocked once the non overlapping region is
		 * cleared and the new memory settings take effect.
		 */
		tegra_lock_videomem_nonoverlap(video_mem_base,
			video_mem_size_mb << 20);
	}

	/* program the Videomem aperture */
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
			  (uint32_t)(phys_base >> 32));
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, size_in_bytes >> 20);

	/* Redundancy check for Video Protect setting */
	assert(tegra_mc_read_32(MC_VIDEO_PROTECT_BASE_LO)
		 == (uint32_t)phys_base);
	assert(tegra_mc_read_32(MC_VIDEO_PROTECT_BASE_HI)
		 == (uint32_t)(phys_base >> 32));
	assert(tegra_mc_read_32(MC_VIDEO_PROTECT_SIZE_MB)
		 == (size_in_bytes >> 20));

	/*
	 * MCE propagates the VideoMem configuration values across the
	 * CCPLEX.
	 */
	(void)mce_update_gsc_videomem();

	/* Clear the non-overlapping memory */
	if (video_mem_base != 0U) {
		tegra_clear_videomem_nonoverlap(phys_base, size_in_bytes);
		tegra_unlock_videomem_nonoverlap();
	}

	/* store new values */
	video_mem_base = phys_base;
	video_mem_size_mb = (uint64_t)size_in_bytes >> 20;
}

/*
 * This feature exists only for v1 of the Tegra Memory Controller.
 */
void tegra_memctrl_disable_ahb_redirection(void)
{
	; /* do nothing */
}

void tegra_memctrl_clear_pending_interrupts(void)
{
	; /* do nothing */
}
