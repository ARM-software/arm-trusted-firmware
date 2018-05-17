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

/* Video Memory base and size (live values) */
static uint64_t video_mem_base;
static uint64_t video_mem_size_mb;

/*
 * Init Memory controller during boot.
 */
void tegra_memctrl_setup(void)
{
	uint32_t val;
	const uint32_t *mc_streamid_override_regs;
	uint32_t num_streamid_override_regs;
	const mc_streamid_security_cfg_t *mc_streamid_sec_cfgs;
	uint32_t num_streamid_sec_cfgs;
	const tegra_mc_settings_t *plat_mc_settings = tegra_get_mc_settings();
	uint32_t i;

	INFO("Tegra Memory Controller (v2)\n");

	/* Program the SMMU pagesize */
	tegra_smmu_init();

	/* Get the settings from the platform */
	assert(plat_mc_settings != NULL);
	mc_streamid_override_regs = plat_mc_settings->streamid_override_cfg;
	num_streamid_override_regs = plat_mc_settings->num_streamid_override_cfgs;
	mc_streamid_sec_cfgs = plat_mc_settings->streamid_security_cfg;
	num_streamid_sec_cfgs = plat_mc_settings->num_streamid_security_cfgs;

	/* Program all the Stream ID overrides */
	for (i = 0; i < num_streamid_override_regs; i++)
		tegra_mc_streamid_write_32(mc_streamid_override_regs[i],
			MC_STREAM_ID_MAX);

	/* Program the security config settings for all Stream IDs */
	for (i = 0; i < num_streamid_sec_cfgs; i++) {
		val = mc_streamid_sec_cfgs[i].override_enable << 16 |
		      mc_streamid_sec_cfgs[i].override_client_inputs << 8 |
		      mc_streamid_sec_cfgs[i].override_client_ns_flag << 0;
		tegra_mc_streamid_write_32(mc_streamid_sec_cfgs[i].offset, val);
	}

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

	/*
	 * Re-configure MSS to allow ROC to deal with ordering of the
	 * Memory Controller traffic. This is needed as the Memory Controller
	 * boots with MSS having all control, but ROC provides a performance
	 * boost as compared to MSS.
	 */
	if (plat_mc_settings->reconfig_mss_clients != NULL) {
		plat_mc_settings->reconfig_mss_clients();
	}

	/* Program overrides for MC transactions */
	if (plat_mc_settings->set_txn_overrides != NULL) {
		plat_mc_settings->set_txn_overrides();
	}
}

/*
 * Restore Memory Controller settings after "System Suspend"
 */
void tegra_memctrl_restore_settings(void)
{
	const tegra_mc_settings_t *plat_mc_settings = tegra_get_mc_settings();

	assert(plat_mc_settings != NULL);

	/*
	 * Re-configure MSS to allow ROC to deal with ordering of the
	 * Memory Controller traffic. This is needed as the Memory Controller
	 * resets during System Suspend with MSS having all control, but ROC
	 * provides a performance boost as compared to MSS.
	 */
	if (plat_mc_settings->reconfig_mss_clients != NULL) {
		plat_mc_settings->reconfig_mss_clients();
	}

	/* Program overrides for MC transactions */
	if (plat_mc_settings->set_txn_overrides != NULL) {
		plat_mc_settings->set_txn_overrides();
	}

	/* video memory carveout region */
	if (video_mem_base != 0ULL) {
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO,
				  (uint32_t)video_mem_base);
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
				  (uint32_t)(video_mem_base >> 32));
		tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, video_mem_size_mb);

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
	uint32_t index;
	uint32_t total_128kb_blocks = size_in_bytes >> 17;
	uint32_t residual_4kb_blocks = (size_in_bytes & (uint32_t)0x1FFFF) >> 12;
	uint32_t val;

	INFO("Configuring TrustZone SRAM Memory Carveout\n");

	/*
	 * Reset the access configuration registers to restrict access
	 * to the TZRAM aperture
	 */
	for (index = MC_TZRAM_CLIENT_ACCESS0_CFG0;
	     index < ((uint32_t)MC_TZRAM_CARVEOUT_CFG + (uint32_t)MC_GSC_CONFIG_REGS_SIZE);
	     index += 4U) {
		tegra_mc_write_32(index, 0);
	}

	/*
	 * Enable CPU access configuration registers to access the TZRAM aperture
	 */
	if (!tegra_chipid_is_t186()) {
		val = tegra_mc_read_32(MC_TZRAM_CLIENT_ACCESS1_CFG0);
		val |= TZRAM_ALLOW_MPCORER | TZRAM_ALLOW_MPCOREW;
		tegra_mc_write_32(MC_TZRAM_CLIENT_ACCESS1_CFG0, val);
	}

	/*
	 * Set the TZRAM base. TZRAM base must be 4k aligned, at least.
	 */
	assert((phys_base & (uint64_t)0xFFF) == 0U);
	tegra_mc_write_32(MC_TZRAM_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_TZRAM_BASE_HI,
		(uint32_t)(phys_base >> 32) & MC_GSC_BASE_HI_MASK);

	/*
	 * Set the TZRAM size
	 *
	 * total size = (number of 128KB blocks) + (number of remaining 4KB
	 * blocks)
	 *
	 */
	val = (residual_4kb_blocks << MC_GSC_SIZE_RANGE_4KB_SHIFT) |
	      total_128kb_blocks;
	tegra_mc_write_32(MC_TZRAM_SIZE, val);

	/*
	 * Lock the configuration settings by disabling TZ-only lock
	 * and locking the configuration against any future changes
	 * at all.
	 */
	val = tegra_mc_read_32(MC_TZRAM_CARVEOUT_CFG);
	val &= (uint32_t)~MC_GSC_ENABLE_TZ_LOCK_BIT;
	val |= MC_GSC_LOCK_CFG_SETTINGS_BIT;
	if (!tegra_chipid_is_t186()) {
		val |= MC_GSC_ENABLE_CPU_SECURE_BIT;
	}
	tegra_mc_write_32(MC_TZRAM_CARVEOUT_CFG, val);

	/*
	 * MCE propagates the security configuration values across the
	 * CCPLEX.
	 */
	mce_update_gsc_tzram();
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

	/*
	 * Map the NS memory first, clean it and then unmap it.
	 */
	ret = mmap_add_dynamic_region(non_overlap_area_start, /* PA */
				non_overlap_area_start, /* VA */
				non_overlap_area_size, /* size */
				MT_NS | MT_RW | MT_EXECUTE_NEVER); /* attrs */
	assert(ret == 0);

	zero_normalmem((void *)non_overlap_area_start, non_overlap_area_size);
	flush_dcache_range(non_overlap_area_start, non_overlap_area_size);

	(void)mmap_remove_dynamic_region(non_overlap_area_start,
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
	uintptr_t vmem_end_old = video_mem_base + (video_mem_size_mb << 20);
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
	if (video_mem_base == 0U)
		goto done;

	/*
	 * Lock the non overlapping memory being cleared so that other masters
	 * do not accidently write to it. The memory would be unlocked once
	 * the non overlapping region is cleared and the new memory
	 * settings take effect.
	 */
	tegra_lock_videomem_nonoverlap(video_mem_base,
				       video_mem_size_mb << 20);

	/*
	 * Clear the old regions now being exposed. The following cases
	 * can occur -
	 *
	 * 1. clear whole old region (no overlap with new region)
	 * 2. clear old sub-region below new base
	 * 3. clear old sub-region above new end
	 */
	INFO("Cleaning previous Video Memory Carveout\n");

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

done:
	/* program the Videomem aperture */
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
			  (uint32_t)(phys_base >> 32));
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, size_in_bytes >> 20);

	/* unlock the previous locked nonoverlapping aperture */
	tegra_unlock_videomem_nonoverlap();

	/* store new values */
	video_mem_base = phys_base;
	video_mem_size_mb = size_in_bytes >> 20;

	/*
	 * MCE propagates the VideoMem configuration values across the
	 * CCPLEX.
	 */
	mce_update_gsc_videomem();
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
