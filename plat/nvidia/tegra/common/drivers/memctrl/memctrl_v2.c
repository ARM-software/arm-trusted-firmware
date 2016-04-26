/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <xlat_tables.h>

#define TEGRA_GPU_RESET_REG_OFFSET	0x30
#define  GPU_RESET_BIT			(1 << 0)

/* Video Memory base and size (live values) */
static uint64_t video_mem_base;
static uint64_t video_mem_size;

/* array to hold stream_id override config register offsets */
const static uint32_t streamid_overrides[] = {
	MC_STREAMID_OVERRIDE_CFG_PTCR,
	MC_STREAMID_OVERRIDE_CFG_AFIR,
	MC_STREAMID_OVERRIDE_CFG_HDAR,
	MC_STREAMID_OVERRIDE_CFG_HOST1XDMAR,
	MC_STREAMID_OVERRIDE_CFG_NVENCSRD,
	MC_STREAMID_OVERRIDE_CFG_SATAR,
	MC_STREAMID_OVERRIDE_CFG_MPCORER,
	MC_STREAMID_OVERRIDE_CFG_NVENCSWR,
	MC_STREAMID_OVERRIDE_CFG_AFIW,
	MC_STREAMID_OVERRIDE_CFG_SATAW,
	MC_STREAMID_OVERRIDE_CFG_MPCOREW,
	MC_STREAMID_OVERRIDE_CFG_SATAW,
	MC_STREAMID_OVERRIDE_CFG_HDAW,
	MC_STREAMID_OVERRIDE_CFG_ISPRA,
	MC_STREAMID_OVERRIDE_CFG_ISPWA,
	MC_STREAMID_OVERRIDE_CFG_ISPWB,
	MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTR,
	MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTW,
	MC_STREAMID_OVERRIDE_CFG_XUSB_DEVR,
	MC_STREAMID_OVERRIDE_CFG_XUSB_DEVW,
	MC_STREAMID_OVERRIDE_CFG_TSECSRD,
	MC_STREAMID_OVERRIDE_CFG_TSECSWR,
	MC_STREAMID_OVERRIDE_CFG_GPUSRD,
	MC_STREAMID_OVERRIDE_CFG_GPUSWR,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRAA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCR,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRAB,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWAA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCW,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWAB,
	MC_STREAMID_OVERRIDE_CFG_VICSRD,
	MC_STREAMID_OVERRIDE_CFG_VICSWR,
	MC_STREAMID_OVERRIDE_CFG_VIW,
	MC_STREAMID_OVERRIDE_CFG_NVDECSRD,
	MC_STREAMID_OVERRIDE_CFG_NVDECSWR,
	MC_STREAMID_OVERRIDE_CFG_APER,
	MC_STREAMID_OVERRIDE_CFG_APEW,
	MC_STREAMID_OVERRIDE_CFG_NVJPGSRD,
	MC_STREAMID_OVERRIDE_CFG_NVJPGSWR,
	MC_STREAMID_OVERRIDE_CFG_SESRD,
	MC_STREAMID_OVERRIDE_CFG_SESWR,
	MC_STREAMID_OVERRIDE_CFG_ETRR,
	MC_STREAMID_OVERRIDE_CFG_ETRW,
	MC_STREAMID_OVERRIDE_CFG_TSECSRDB,
	MC_STREAMID_OVERRIDE_CFG_TSECSWRB,
	MC_STREAMID_OVERRIDE_CFG_GPUSRD2,
	MC_STREAMID_OVERRIDE_CFG_GPUSWR2,
	MC_STREAMID_OVERRIDE_CFG_AXISR,
	MC_STREAMID_OVERRIDE_CFG_AXISW,
	MC_STREAMID_OVERRIDE_CFG_EQOSR,
	MC_STREAMID_OVERRIDE_CFG_EQOSW,
	MC_STREAMID_OVERRIDE_CFG_UFSHCR,
	MC_STREAMID_OVERRIDE_CFG_UFSHCW,
	MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR,
	MC_STREAMID_OVERRIDE_CFG_BPMPR,
	MC_STREAMID_OVERRIDE_CFG_BPMPW,
	MC_STREAMID_OVERRIDE_CFG_BPMPDMAR,
	MC_STREAMID_OVERRIDE_CFG_BPMPDMAW,
	MC_STREAMID_OVERRIDE_CFG_AONR,
	MC_STREAMID_OVERRIDE_CFG_AONW,
	MC_STREAMID_OVERRIDE_CFG_AONDMAR,
	MC_STREAMID_OVERRIDE_CFG_AONDMAW,
	MC_STREAMID_OVERRIDE_CFG_SCER,
	MC_STREAMID_OVERRIDE_CFG_SCEW,
	MC_STREAMID_OVERRIDE_CFG_SCEDMAR,
	MC_STREAMID_OVERRIDE_CFG_SCEDMAW,
	MC_STREAMID_OVERRIDE_CFG_APEDMAR,
	MC_STREAMID_OVERRIDE_CFG_APEDMAW,
	MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR1,
	MC_STREAMID_OVERRIDE_CFG_VICSRD1,
	MC_STREAMID_OVERRIDE_CFG_NVDECSRD1
};

/* array to hold the security configs for stream IDs */
const static mc_streamid_security_cfg_t sec_cfgs[] = {
	mc_make_sec_cfg(SCEW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(AFIR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDISPLAYR1, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_DEVR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(VICSRD1, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVENCSWR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(TSECSRDB, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(AXISW, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCWAB, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(AONDMAW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(GPUSWR2, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SATAW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(UFSHCW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(AFIW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SDMMCR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SCEDMAW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(UFSHCR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SDMMCWAA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(APEDMAW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(SESWR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(MPCORER, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(PTCR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(BPMPW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(ETRW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(GPUSRD, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(VICSWR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(SCEDMAR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(HDAW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(ISPWA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(EQOSW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_HOSTW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(TSECSWR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(SDMMCRAA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(APER, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(VIW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(APEW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(AXISR, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(BPMPDMAW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(ISPRA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDECSWR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_DEVW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDECSRD, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(MPCOREW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDISPLAYR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(BPMPDMAR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVJPGSWR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDECSRD1, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(TSECSRD, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVJPGSRD, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(SDMMCWA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SCER, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_HOSTR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(VICSRD, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(AONDMAR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(AONW, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SDMMCRA, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(HOST1XDMAR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(EQOSR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SATAR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(BPMPR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(HDAR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(SDMMCRAB, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(ETRR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(AONR, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(APEDMAR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(SESRD, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVENCSRD, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(GPUSWR, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSWRB, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(ISPWB, NON_SECURE, OVERRIDE, ENABLE),
	mc_make_sec_cfg(GPUSRD2, SECURE, NO_OVERRIDE, DISABLE),
};

const static mc_txn_override_cfg_t mc_override_cfgs[] = {
	mc_make_txn_override_cfg(BPMPW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(EQOSW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(NVJPGSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCWAA, CGID_TAG_ADR),
	mc_make_txn_override_cfg(MPCOREW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SCEDMAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AXISW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(TSECSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(GPUSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(XUSB_HOSTW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(TSECSWRB, CGID_TAG_ADR),
	mc_make_txn_override_cfg(GPUSWR2, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AONDMAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AONW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SESWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(BPMPDMAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCWA, CGID_TAG_ADR),
	mc_make_txn_override_cfg(HDAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(NVDECSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(UFSHCW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SATAW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(ETRW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(VICSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(NVENCSWR, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SDMMCWAB, CGID_TAG_ADR),
	mc_make_txn_override_cfg(ISPWB, CGID_TAG_ADR),
	mc_make_txn_override_cfg(APEW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(XUSB_DEVW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(AFIW, CGID_TAG_ADR),
	mc_make_txn_override_cfg(SCEW, CGID_TAG_ADR),
};

/*
 * Init Memory controller during boot.
 */
void tegra_memctrl_setup(void)
{
	uint32_t val;
	uint32_t num_overrides = sizeof(streamid_overrides) / sizeof(uint32_t);
	uint32_t num_sec_cfgs = sizeof(sec_cfgs) / sizeof(mc_streamid_security_cfg_t);
	uint32_t num_txn_overrides = sizeof(mc_override_cfgs) / sizeof(mc_txn_override_cfg_t);
	uint32_t tegra_rev;
	int i;

	INFO("Tegra Memory Controller (v2)\n");

	/* Program the SMMU pagesize */
	tegra_smmu_init();

	/* Program all the Stream ID overrides */
	for (i = 0; i < num_overrides; i++)
		tegra_mc_streamid_write_32(streamid_overrides[i],
			MC_STREAM_ID_MAX);

	/* Program the security config settings for all Stream IDs */
	for (i = 0; i < num_sec_cfgs; i++) {
		val = sec_cfgs[i].override_enable << 16 |
		      sec_cfgs[i].override_client_inputs << 8 |
		      sec_cfgs[i].override_client_ns_flag << 0;
		tegra_mc_streamid_write_32(sec_cfgs[i].offset, val);
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
	 * Set the MC_TXN_OVERRIDE registers for write clients.
	 */
	tegra_rev = (mmio_read_32(TEGRA_MISC_BASE + HARDWARE_REVISION_OFFSET) &
			HARDWARE_MINOR_REVISION_MASK) >> HARDWARE_MINOR_REVISION_SHIFT;

	if (tegra_rev == HARDWARE_REVISION_A01) {

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
		for (i = 0; i < num_txn_overrides; i++) {
			val = tegra_mc_read_32(mc_override_cfgs[i].offset);
			val &= ~MC_TXN_OVERRIDE_CGID_TAG_MASK;
			tegra_mc_write_32(mc_override_cfgs[i].offset,
				val | mc_override_cfgs[i].cgid_tag);
		}

	}
}

/*
 * Restore Memory Controller settings after "System Suspend"
 */
void tegra_memctrl_restore_settings(void)
{
	/* video memory carveout region */
	if (video_mem_base) {
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_LO,
				  (uint32_t)video_mem_base);
		tegra_mc_write_32(MC_VIDEO_PROTECT_BASE_HI,
				  (uint32_t)(video_mem_base >> 32));
		tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, video_mem_size);

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
	uint64_t tzram_end = phys_base + size_in_bytes - 1;
	uint32_t val;

	/*
	 * Check if the TZRAM is locked already.
	 */
	if (tegra_mc_read_32(MC_TZRAM_REG_CTRL) == DISABLE_TZRAM_ACCESS)
		return;

	/*
	 * Setup the Memory controller to allow only secure accesses to
	 * the TZRAM carveout
	 */
	INFO("Configuring TrustZone RAM (SysRAM) Memory Carveout\n");

	/* Program the base and end values */
	tegra_mc_write_32(MC_TZRAM_BASE, (uint32_t)phys_base);
	tegra_mc_write_32(MC_TZRAM_END, (uint32_t)tzram_end);

	/* Extract the high address bits from the base/end values */
	val = (uint32_t)(phys_base >> 32) & TZRAM_ADDR_HI_BITS_MASK;
	val |= (((uint32_t)(tzram_end >> 32) & TZRAM_ADDR_HI_BITS_MASK) <<
		TZRAM_END_HI_BITS_SHIFT);
	tegra_mc_write_32(MC_TZRAM_HI_ADDR_BITS, val);

	/* Disable further writes to the TZRAM setup registers */
	tegra_mc_write_32(MC_TZRAM_REG_CTRL, DISABLE_TZRAM_ACCESS);

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
	tegra_mc_write_32(MC_VIDEO_PROTECT_SIZE_MB, size_in_bytes);

	/* store new values */
	video_mem_base = phys_base;
	video_mem_size = size_in_bytes >> 20;

	/*
	 * MCE propogates the VideoMem configuration values across the
	 * CCPLEX.
	 */
	mce_update_gsc_videomem();
}
