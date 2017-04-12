/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
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
#include <mce.h>
#include <memctrl.h>
#include <memctrl_v2.h>
#include <mmio.h>
#include <smmu.h>
#include <string.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <xlat_tables.h>

#define TEGRA_GPU_RESET_REG_OFFSET	0x30
#define  GPU_RESET_BIT			(1 << 0)

/* Video Memory base and size (live values) */
static uint64_t video_mem_base;
static uint64_t video_mem_size_mb;

static void tegra_memctrl_reconfig_mss_clients(void)
{
#if ENABLE_ROC_FOR_ORDERING_CLIENT_REQUESTS
	uint32_t val, wdata_0, wdata_1;

	/*
	 * Assert Memory Controller's HOTRESET_FLUSH_ENABLE signal for
	 * boot and strongly ordered MSS clients to flush existing memory
	 * traffic and stall future requests.
	 */
	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL0);
	assert(val == MC_CLIENT_HOTRESET_CTRL0_RESET_VAL);

	wdata_0 = MC_CLIENT_HOTRESET_CTRL0_HDA_FLUSH_ENB |
#if ENABLE_AFI_DEVICE
		  MC_CLIENT_HOTRESET_CTRL0_AFI_FLUSH_ENB |
#endif
		  MC_CLIENT_HOTRESET_CTRL0_SATA_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_XUSB_HOST_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_XUSB_DEV_FLUSH_ENB;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL0, wdata_0);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((val & wdata_0) != wdata_0);

	/* Wait one more time due to SW WAR for known legacy issue */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((val & wdata_0) != wdata_0);

	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL1);
	assert(val == MC_CLIENT_HOTRESET_CTRL1_RESET_VAL);

	wdata_1 = MC_CLIENT_HOTRESET_CTRL1_SDMMC4A_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_APE_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_SE_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_ETR_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_AXIS_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_EQOS_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_UFSHC_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_BPMP_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_AON_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_SCE_FLUSH_ENB;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL1, wdata_1);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((val & wdata_1) != wdata_1);

	/* Wait one more time due to SW WAR for known legacy issue */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((val & wdata_1) != wdata_1);

	/*
	 * Change MEMTYPE_OVERRIDE from SO_DEV -> PASSTHRU for boot and
	 * strongly ordered MSS clients. ROC needs to be single point
	 * of control on overriding the memory type. So, remove TSA's
	 * memtype override.
	 */
#if ENABLE_AFI_DEVICE
	mc_set_tsa_passthrough(AFIW);
#endif
	mc_set_tsa_passthrough(HDAW);
	mc_set_tsa_passthrough(SATAW);
	mc_set_tsa_passthrough(XUSB_HOSTW);
	mc_set_tsa_passthrough(XUSB_DEVW);
	mc_set_tsa_passthrough(SDMMCWAB);
	mc_set_tsa_passthrough(APEDMAW);
	mc_set_tsa_passthrough(SESWR);
	mc_set_tsa_passthrough(ETRW);
	mc_set_tsa_passthrough(AXISW);
	mc_set_tsa_passthrough(EQOSW);
	mc_set_tsa_passthrough(UFSHCW);
	mc_set_tsa_passthrough(BPMPDMAW);
	mc_set_tsa_passthrough(AONDMAW);
	mc_set_tsa_passthrough(SCEDMAW);

	/*
	 * Change COH_PATH_OVERRIDE_SO_DEV from NO_OVERRIDE -> FORCE_COHERENT
	 * for boot and strongly ordered MSS clients. This steers all sodev
	 * transactions to ROC.
	 *
	 * Change AXID_OVERRIDE/AXID_OVERRIDE_SO_DEV only for some clients
	 * whose AXI IDs we know and trust.
	 */

#if ENABLE_AFI_DEVICE
	/* Match AFIW */
	mc_set_forced_coherent_so_dev_cfg(AFIR);
#endif

	/*
	 * See bug 200131110 comment #35 - there are no normal requests
	 * and AWID for SO/DEV requests is hardcoded in RTL for a
	 * particular PCIE controller
	 */
#if ENABLE_AFI_DEVICE
	mc_set_forced_coherent_so_dev_cfg(AFIW);
#endif
	mc_set_forced_coherent_cfg(HDAR);
	mc_set_forced_coherent_cfg(HDAW);
	mc_set_forced_coherent_cfg(SATAR);
	mc_set_forced_coherent_cfg(SATAW);
	mc_set_forced_coherent_cfg(XUSB_HOSTR);
	mc_set_forced_coherent_cfg(XUSB_HOSTW);
	mc_set_forced_coherent_cfg(XUSB_DEVR);
	mc_set_forced_coherent_cfg(XUSB_DEVW);
	mc_set_forced_coherent_cfg(SDMMCRAB);
	mc_set_forced_coherent_cfg(SDMMCWAB);

	/* Match APEDMAW */
	mc_set_forced_coherent_axid_so_dev_cfg(APEDMAR);

	/*
	 * See bug 200131110 comment #35 - AWID for normal requests
	 * is 0x80 and AWID for SO/DEV requests is 0x01
	 */
	mc_set_forced_coherent_axid_so_dev_cfg(APEDMAW);
	mc_set_forced_coherent_cfg(SESRD);
	mc_set_forced_coherent_cfg(SESWR);
	mc_set_forced_coherent_cfg(ETRR);
	mc_set_forced_coherent_cfg(ETRW);
	mc_set_forced_coherent_cfg(AXISR);
	mc_set_forced_coherent_cfg(AXISW);
	mc_set_forced_coherent_cfg(EQOSR);
	mc_set_forced_coherent_cfg(EQOSW);
	mc_set_forced_coherent_cfg(UFSHCR);
	mc_set_forced_coherent_cfg(UFSHCW);
	mc_set_forced_coherent_cfg(BPMPDMAR);
	mc_set_forced_coherent_cfg(BPMPDMAW);
	mc_set_forced_coherent_cfg(AONDMAR);
	mc_set_forced_coherent_cfg(AONDMAW);
	mc_set_forced_coherent_cfg(SCEDMAR);
	mc_set_forced_coherent_cfg(SCEDMAW);

	/*
	 * At this point, ordering can occur at ROC. So, remove PCFIFO's
	 * control over ordering requests.
	 *
	 * Change PCFIFO_*_ORDERED_CLIENT from ORDERED -> UNORDERED for
	 * boot and strongly ordered MSS clients
	 */
	val = MC_PCFIFO_CLIENT_CONFIG1_RESET_VAL &
#if ENABLE_AFI_DEVICE
		mc_set_pcfifo_unordered_boot_so_mss(1, AFIW) &
#endif
		mc_set_pcfifo_unordered_boot_so_mss(1, HDAW) &
		mc_set_pcfifo_unordered_boot_so_mss(1, SATAW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG1, val);

	val = MC_PCFIFO_CLIENT_CONFIG2_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(2, XUSB_HOSTW) &
		mc_set_pcfifo_unordered_boot_so_mss(2, XUSB_DEVW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG2, val);

	val = MC_PCFIFO_CLIENT_CONFIG3_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(3, SDMMCWAB);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG3, val);

	val = MC_PCFIFO_CLIENT_CONFIG4_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(4, SESWR) &
		mc_set_pcfifo_unordered_boot_so_mss(4, ETRW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AXISW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, EQOSW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, UFSHCW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, BPMPDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AONDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, SCEDMAW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG4, val);

	val = MC_PCFIFO_CLIENT_CONFIG5_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(5, APEDMAW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG5, val);

	/*
	 * At this point, ordering can occur at ROC. SMMU need not
	 * reorder any requests.
	 *
	 * Change SMMU_*_ORDERED_CLIENT from ORDERED -> UNORDERED
	 * for boot and strongly ordered MSS clients
	 */
	val = MC_SMMU_CLIENT_CONFIG1_RESET_VAL &
#if ENABLE_AFI_DEVICE
		mc_set_smmu_unordered_boot_so_mss(1, AFIW) &
#endif
		mc_set_smmu_unordered_boot_so_mss(1, HDAW) &
		mc_set_smmu_unordered_boot_so_mss(1, SATAW);
	tegra_mc_write_32(MC_SMMU_CLIENT_CONFIG1, val);

	val = MC_SMMU_CLIENT_CONFIG2_RESET_VAL &
		mc_set_smmu_unordered_boot_so_mss(2, XUSB_HOSTW) &
		mc_set_smmu_unordered_boot_so_mss(2, XUSB_DEVW);
	tegra_mc_write_32(MC_SMMU_CLIENT_CONFIG2, val);

	val = MC_SMMU_CLIENT_CONFIG3_RESET_VAL &
		mc_set_smmu_unordered_boot_so_mss(3, SDMMCWAB);
	tegra_mc_write_32(MC_SMMU_CLIENT_CONFIG3, val);

	val = MC_SMMU_CLIENT_CONFIG4_RESET_VAL &
		mc_set_smmu_unordered_boot_so_mss(4, SESWR) &
		mc_set_smmu_unordered_boot_so_mss(4, ETRW) &
		mc_set_smmu_unordered_boot_so_mss(4, AXISW) &
		mc_set_smmu_unordered_boot_so_mss(4, EQOSW) &
		mc_set_smmu_unordered_boot_so_mss(4, UFSHCW) &
		mc_set_smmu_unordered_boot_so_mss(4, BPMPDMAW) &
		mc_set_smmu_unordered_boot_so_mss(4, AONDMAW) &
		mc_set_smmu_unordered_boot_so_mss(4, SCEDMAW);
	tegra_mc_write_32(MC_SMMU_CLIENT_CONFIG4, val);

	val = MC_SMMU_CLIENT_CONFIG5_RESET_VAL &
		mc_set_smmu_unordered_boot_so_mss(5, APEDMAW);
	tegra_mc_write_32(MC_SMMU_CLIENT_CONFIG5, val);

	/*
	 * Deassert HOTRESET FLUSH_ENABLE for boot and strongly ordered MSS
	 * clients to allow memory traffic from all clients to start passing
	 * through ROC
	 */
	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL0);
	assert(val == wdata_0);

	wdata_0 = MC_CLIENT_HOTRESET_CTRL0_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL0, wdata_0);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((val & wdata_0) != wdata_0);

	/* Wait one more time due to SW WAR for known legacy issue */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((val & wdata_0) != wdata_0);

	val = tegra_mc_read_32(MC_CLIENT_HOTRESET_CTRL1);
	assert(val == wdata_1);

	wdata_1 = MC_CLIENT_HOTRESET_CTRL1_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL1, wdata_1);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((val & wdata_1) != wdata_1);

	/* Wait one more time due to SW WAR for known legacy issue */
	do {
		val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((val & wdata_1) != wdata_1);

#endif
}

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
	const mc_txn_override_cfg_t *mc_txn_override_cfgs;
	uint32_t num_txn_override_cfgs;
	tegra_mc_settings_t *plat_mc_settings = tegra_get_mc_settings();
	int i;

	INFO("Tegra Memory Controller (v2)\n");

#if ENABLE_SMMU_DEVICE
	/* Program the SMMU pagesize */
	tegra_smmu_init();
#endif
	/* Get the settings from the platform */
	assert(plat_mc_settings);
	mc_streamid_override_regs = plat_mc_settings->streamid_override_cfg;
	num_streamid_override_regs = plat_mc_settings->num_streamid_override_cfgs;
	mc_streamid_sec_cfgs = plat_mc_settings->streamid_security_cfg;
	num_streamid_sec_cfgs = plat_mc_settings->num_streamid_security_cfgs;
	mc_txn_override_cfgs = plat_mc_settings->txn_override_cfg;
	num_txn_override_cfgs = plat_mc_settings->num_txn_override_cfgs;

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
	tegra_memctrl_reconfig_mss_clients();

	/*
	 * Set the MC_TXN_OVERRIDE registers for write clients.
	 */
	if (!tegra_platform_is_silicon() ||
	    (tegra_platform_is_silicon() && tegra_get_chipid_minor() == 1)) {

		/* GPU and NVENC settings for rev. A01 */
		val = tegra_mc_read_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR);
		val &= ~MC_TXN_OVERRIDE_CGID_TAG_MASK;
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR,
			val | MC_TXN_OVERRIDE_CGID_TAG_ZERO);

		val = tegra_mc_read_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR2);
		val &= ~MC_TXN_OVERRIDE_CGID_TAG_MASK;
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_GPUSWR2,
			val | MC_TXN_OVERRIDE_CGID_TAG_ZERO);

		val = tegra_mc_read_32(MC_TXN_OVERRIDE_CONFIG_NVENCSWR);
		val &= ~MC_TXN_OVERRIDE_CGID_TAG_MASK;
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_NVENCSWR,
			val | MC_TXN_OVERRIDE_CGID_TAG_CLIENT_AXI_ID);

	} else {

		/* settings for rev. A02 */
		for (i = 0; i < num_txn_override_cfgs; i++) {
			val = tegra_mc_read_32(mc_txn_override_cfgs[i].offset);
			val &= ~MC_TXN_OVERRIDE_CGID_TAG_MASK;
			tegra_mc_write_32(mc_txn_override_cfgs[i].offset,
				val | mc_txn_override_cfgs[i].cgid_tag);
		}

	}
}

/*
 * Restore Memory Controller settings after "System Suspend"
 */
void tegra_memctrl_restore_settings(void)
{
	/*
	 * Re-configure MSS to allow ROC to deal with ordering of the
	 * Memory Controller traffic. This is needed as the Memory Controller
	 * resets during System Suspend with MSS having all control, but ROC
	 * provides a performance boost as compared to MSS.
	 */
	tegra_memctrl_reconfig_mss_clients();

	/* video memory carveout region */
	if (video_mem_base) {
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO,
				  (uint32_t)video_mem_base);
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
				  (uint32_t)(video_mem_base >> 32));
		tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, video_mem_size_mb);

		/*
		 * MCE propogates the VideoMem configuration values across the
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
	 * Setup the Memory controller to allow only secure accesses to
	 * the TZDRAM carveout
	 */
	INFO("Configuring TrustZone DRAM Memory Carveout\n");

	tegra_mc_write_32(MC_SECURITY_CFG0_0, (uint32_t)phys_base);
	tegra_mc_write_32(MC_SECURITY_CFG3_0, (uint32_t)(phys_base >> 32));
	tegra_mc_write_32(MC_SECURITY_CFG1_0, size_in_bytes >> 20);

	/*
	 * When TZ encryption enabled,
	 * We need setup TZDRAM before CPU to access TZ Carveout,
	 * otherwise CPU will fetch non-decrypted data.
	 * So save TZDRAM setting for retore by SC7 resume FW.
	 */

	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV55_LO,
					tegra_mc_read_32(MC_SECURITY_CFG0_0));
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV55_HI,
					tegra_mc_read_32(MC_SECURITY_CFG3_0));
	mmio_write_32(TEGRA_SCRATCH_BASE + SECURE_SCRATCH_RSV54_HI,
					tegra_mc_read_32(MC_SECURITY_CFG1_0));

	/*
	 * MCE propogates the security configuration values across the
	 * CCPLEX.
	 */
	mce_update_gsc_tzdram();
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
	uint32_t residual_4kb_blocks = (size_in_bytes & 0x1FFFF) >> 12;
	uint32_t val;

	/*
	 * Reset the access configuration registers to restrict access
	 * to the TZRAM aperture
	 */
	for (index = MC_TZRAM_CARVEOUT_CLIENT_ACCESS_CFG0;
	     index <= MC_TZRAM_CARVEOUT_FORCE_INTERNAL_ACCESS5;
	     index += 4)
		tegra_mc_write_32(index, 0);

	/*
	 * Set the TZRAM base. TZRAM base must be 4k aligned, at least.
	 */
	assert(!(phys_base & 0xFFF));
	tegra_mc_write_32(MC_TZRAM_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_TZRAM_BASE_HI,
		(uint32_t)(phys_base >> 32) & TZRAM_BASE_HI_MASK);

	/*
	 * Set the TZRAM size
	 *
	 * total size = (number of 128KB blocks) + (number of remaining 4KB
	 * blocks)
	 *
	 */
	val = (residual_4kb_blocks << TZRAM_SIZE_RANGE_4KB_SHIFT) |
	      total_128kb_blocks;
	tegra_mc_write_32(MC_TZRAM_SIZE, val);

	/*
	 * Lock the configuration settings by disabling TZ-only lock
	 * and locking the configuration against any future changes
	 * at all.
	 */
	val = tegra_mc_read_32(MC_TZRAM_CARVEOUT_CFG);
	val &= ~TZRAM_ENABLE_TZ_LOCK_BIT;
	val |= TZRAM_LOCK_CFG_SETTINGS_BIT;
	tegra_mc_write_32(MC_TZRAM_CARVEOUT_CFG, val);

	/*
	 * MCE propogates the security configuration values across the
	 * CCPLEX.
	 */
	mce_update_gsc_tzram();
}

/*
 * Program the Video Memory carveout region
 *
 * phys_base = physical base of aperture
 * size_in_bytes = size of aperture in bytes
 */
void tegra_memctrl_videomem_setup(uint64_t phys_base, uint32_t size_in_bytes)
{
	uint32_t regval;

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

	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO, (uint32_t)phys_base);
	tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
			  (uint32_t)(phys_base >> 32));
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, size_in_bytes >> 20);

	/* store new values */
	video_mem_base = phys_base;
	video_mem_size_mb = size_in_bytes >> 20;

	/*
	 * MCE propogates the VideoMem configuration values across the
	 * CCPLEX.
	 */
	mce_update_gsc_videomem();
}
