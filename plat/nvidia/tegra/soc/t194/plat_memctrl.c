/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <mce.h>
#include <memctrl_v2.h>
#include <tegra_mc_def.h>
#include <tegra_platform.h>

/*******************************************************************************
 * Array to hold the security configs for stream IDs
 ******************************************************************************/
const static mc_streamid_security_cfg_t tegra194_streamid_sec_cfgs[] = {
	mc_make_sec_cfg(PTCR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(HDAR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(HOST1XDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SATAR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MPCORER, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(HDAW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MPCOREW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SATAW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(ISPRA, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(ISPFALR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(ISPWA, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(ISPWB, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_HOSTR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_HOSTW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_DEVR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(XUSB_DEVW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(TSECSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCRA, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCRAB, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCWA, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(SDMMCWAB, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(VICSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(VICSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(VIW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(NVDECSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDECSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APER, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APEW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVJPGSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVJPGSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SESRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SESWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AXIAPR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(AXIAPW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(ETRR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(ETRW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSRDB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(TSECSWRB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AXISR, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AXISW, SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(EQOSR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(EQOSW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(UFSHCR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(UFSHCW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDISPLAYR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(BPMPDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(AONDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCER, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCEW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCEDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SCEDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APEDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(APEDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDISPLAYR1, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(VICSRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDECSRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(VIFALR, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(VIFALW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(DLA0RDA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA0FALRDB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA0WRA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA0FALWRB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA1RDA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA1FALRDB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA1WRA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA1FALWRB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0RDA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0RDB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0RDC, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0WRA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0WRB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0WRC, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1RDA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1RDB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1RDC, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1WRA, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1WRB, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1WRC, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(RCER, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(RCEW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(RCEDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(RCEDMAW, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENC1SRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENC1SWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE0R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE0W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE1R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE1W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE2AR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE2AW, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE3R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE3W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE4R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE4W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE5R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE5W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(ISPFALW, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(DLA0RDA1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(DLA1RDA1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0RDA1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA0RDB1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1RDA1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PVA1RDB1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(PCIE5R1, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENC1SRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(ISPRA1, NON_SECURE, NO_OVERRIDE, ENABLE),
	mc_make_sec_cfg(PCIE0R1, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDEC1SRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDEC1SRD1, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVDEC1SWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU0R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU0W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU1R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU1W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU2R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU2W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU3R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU3W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU4R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU4W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU5R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU5W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU6R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU6W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU7R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU7W, NON_SECURE, OVERRIDE, DISABLE)
};

/*******************************************************************************
 * Array to hold MC context for Tegra194
 ******************************************************************************/
static __attribute__((aligned(16))) mc_regs_t tegra194_mc_context[] = {
	_START_OF_TABLE_,
	mc_smmu_bypass_cfg,	/* TBU settings */
	_END_OF_TABLE_,
};

/*******************************************************************************
 * Handler to return the pointer to the MC's context struct
 ******************************************************************************/
static mc_regs_t *tegra194_get_mc_system_suspend_ctx(void)
{
	/* index of _END_OF_TABLE_ */
	tegra194_mc_context[0].val = (uint32_t)ARRAY_SIZE(tegra194_mc_context) - 1U;

	return tegra194_mc_context;
}

/*******************************************************************************
 * Struct to hold the memory controller settings
 ******************************************************************************/
static tegra_mc_settings_t tegra194_mc_settings = {
	.streamid_security_cfg = tegra194_streamid_sec_cfgs,
	.num_streamid_security_cfgs = (uint32_t)ARRAY_SIZE(tegra194_streamid_sec_cfgs),
	.get_mc_system_suspend_ctx = tegra194_get_mc_system_suspend_ctx
};

/*******************************************************************************
 * Handler to return the pointer to the memory controller's settings struct
 ******************************************************************************/
tegra_mc_settings_t *tegra_get_mc_settings(void)
{
	return &tegra194_mc_settings;
}

/*******************************************************************************
 * Handler to program the scratch registers with TZDRAM settings for the
 * resume firmware
 ******************************************************************************/
void plat_memctrl_tzdram_setup(uint64_t phys_base, uint64_t size_in_bytes)
{
	uint32_t sec_reg_ctrl = tegra_mc_read_32(MC_SECURITY_CFG_REG_CTRL_0);
	uint32_t phys_base_lo = (uint32_t)phys_base & 0xFFF00000;
	uint32_t phys_base_hi = (uint32_t)(phys_base >> 32);

	/*
	 * Check TZDRAM carveout register access status. Setup TZDRAM fence
	 * only if access is enabled.
	 */
	if ((sec_reg_ctrl & SECURITY_CFG_WRITE_ACCESS_BIT) ==
	     SECURITY_CFG_WRITE_ACCESS_ENABLE) {

		/*
		 * Setup the Memory controller to allow only secure accesses to
		 * the TZDRAM carveout
		 */
		INFO("Configuring TrustZone DRAM Memory Carveout\n");

		tegra_mc_write_32(MC_SECURITY_CFG0_0, phys_base_lo);
		tegra_mc_write_32(MC_SECURITY_CFG3_0, phys_base_hi);
		tegra_mc_write_32(MC_SECURITY_CFG1_0, (uint32_t)(size_in_bytes >> 20));

		/*
		 * MCE propagates the security configuration values across the
		 * CCPLEX.
		 */
		(void)mce_update_gsc_tzdram();
	}
}
