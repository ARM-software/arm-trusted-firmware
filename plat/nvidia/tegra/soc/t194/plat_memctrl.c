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
 * Array to hold stream_id override config register offsets
 ******************************************************************************/
const static uint32_t tegra194_streamid_override_regs[] = {
	MC_STREAMID_OVERRIDE_CFG_HDAR,
	MC_STREAMID_OVERRIDE_CFG_HOST1XDMAR,
	MC_STREAMID_OVERRIDE_CFG_NVENCSRD,
	MC_STREAMID_OVERRIDE_CFG_SATAR,
	MC_STREAMID_OVERRIDE_CFG_NVENCSWR,
	MC_STREAMID_OVERRIDE_CFG_HDAW,
	MC_STREAMID_OVERRIDE_CFG_SATAW,
	MC_STREAMID_OVERRIDE_CFG_ISPRA,
	MC_STREAMID_OVERRIDE_CFG_ISPFALR,
	MC_STREAMID_OVERRIDE_CFG_ISPWA,
	MC_STREAMID_OVERRIDE_CFG_ISPWB,
	MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTR,
	MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTW,
	MC_STREAMID_OVERRIDE_CFG_XUSB_DEVR,
	MC_STREAMID_OVERRIDE_CFG_XUSB_DEVW,
	MC_STREAMID_OVERRIDE_CFG_TSECSRD,
	MC_STREAMID_OVERRIDE_CFG_TSECSWR,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRA,
	MC_STREAMID_OVERRIDE_CFG_SDMMCR,
	MC_STREAMID_OVERRIDE_CFG_SDMMCRAB,
	MC_STREAMID_OVERRIDE_CFG_SDMMCWA,
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
	MC_STREAMID_OVERRIDE_CFG_AXIAPR,
	MC_STREAMID_OVERRIDE_CFG_AXIAPW,
	MC_STREAMID_OVERRIDE_CFG_ETRR,
	MC_STREAMID_OVERRIDE_CFG_ETRW,
	MC_STREAMID_OVERRIDE_CFG_TSECSRDB,
	MC_STREAMID_OVERRIDE_CFG_TSECSWRB,
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
	MC_STREAMID_OVERRIDE_CFG_NVDECSRD1,
	MC_STREAMID_OVERRIDE_CFG_VIFALR,
	MC_STREAMID_OVERRIDE_CFG_VIFALW,
	MC_STREAMID_OVERRIDE_CFG_DLA0RDA,
	MC_STREAMID_OVERRIDE_CFG_DLA0FALRDB,
	MC_STREAMID_OVERRIDE_CFG_DLA0WRA,
	MC_STREAMID_OVERRIDE_CFG_DLA0FALWRB,
	MC_STREAMID_OVERRIDE_CFG_DLA1RDA,
	MC_STREAMID_OVERRIDE_CFG_DLA1FALRDB,
	MC_STREAMID_OVERRIDE_CFG_DLA1WRA,
	MC_STREAMID_OVERRIDE_CFG_DLA1FALWRB,
	MC_STREAMID_OVERRIDE_CFG_PVA0RDA,
	MC_STREAMID_OVERRIDE_CFG_PVA0RDB,
	MC_STREAMID_OVERRIDE_CFG_PVA0RDC,
	MC_STREAMID_OVERRIDE_CFG_PVA0WRA,
	MC_STREAMID_OVERRIDE_CFG_PVA0WRB,
	MC_STREAMID_OVERRIDE_CFG_PVA0WRC,
	MC_STREAMID_OVERRIDE_CFG_PVA1RDA,
	MC_STREAMID_OVERRIDE_CFG_PVA1RDB,
	MC_STREAMID_OVERRIDE_CFG_PVA1RDC,
	MC_STREAMID_OVERRIDE_CFG_PVA1WRA,
	MC_STREAMID_OVERRIDE_CFG_PVA1WRB,
	MC_STREAMID_OVERRIDE_CFG_PVA1WRC,
	MC_STREAMID_OVERRIDE_CFG_RCER,
	MC_STREAMID_OVERRIDE_CFG_RCEW,
	MC_STREAMID_OVERRIDE_CFG_RCEDMAR,
	MC_STREAMID_OVERRIDE_CFG_RCEDMAW,
	MC_STREAMID_OVERRIDE_CFG_NVENC1SRD,
	MC_STREAMID_OVERRIDE_CFG_NVENC1SWR,
	MC_STREAMID_OVERRIDE_CFG_PCIE0R,
	MC_STREAMID_OVERRIDE_CFG_PCIE0W,
	MC_STREAMID_OVERRIDE_CFG_PCIE1R,
	MC_STREAMID_OVERRIDE_CFG_PCIE1W,
	MC_STREAMID_OVERRIDE_CFG_PCIE2AR,
	MC_STREAMID_OVERRIDE_CFG_PCIE2AW,
	MC_STREAMID_OVERRIDE_CFG_PCIE3R,
	MC_STREAMID_OVERRIDE_CFG_PCIE3W,
	MC_STREAMID_OVERRIDE_CFG_PCIE4R,
	MC_STREAMID_OVERRIDE_CFG_PCIE4W,
	MC_STREAMID_OVERRIDE_CFG_PCIE5R,
	MC_STREAMID_OVERRIDE_CFG_PCIE5W,
	MC_STREAMID_OVERRIDE_CFG_ISPFALW,
	MC_STREAMID_OVERRIDE_CFG_DLA0RDA1,
	MC_STREAMID_OVERRIDE_CFG_DLA1RDA1,
	MC_STREAMID_OVERRIDE_CFG_PVA0RDA1,
	MC_STREAMID_OVERRIDE_CFG_PVA0RDB1,
	MC_STREAMID_OVERRIDE_CFG_PVA1RDA1,
	MC_STREAMID_OVERRIDE_CFG_PVA1RDB1,
	MC_STREAMID_OVERRIDE_CFG_PCIE5R1,
	MC_STREAMID_OVERRIDE_CFG_NVENCSRD1,
	MC_STREAMID_OVERRIDE_CFG_NVENC1SRD1,
	MC_STREAMID_OVERRIDE_CFG_ISPRA1,
	MC_STREAMID_OVERRIDE_CFG_PCIE0R1,
	MC_STREAMID_OVERRIDE_CFG_MIU0R,
	MC_STREAMID_OVERRIDE_CFG_MIU0W,
	MC_STREAMID_OVERRIDE_CFG_MIU1R,
	MC_STREAMID_OVERRIDE_CFG_MIU1W,
	MC_STREAMID_OVERRIDE_CFG_MIU2R,
	MC_STREAMID_OVERRIDE_CFG_MIU2W,
	MC_STREAMID_OVERRIDE_CFG_MIU3R,
	MC_STREAMID_OVERRIDE_CFG_MIU3W
};

/*******************************************************************************
 * Array to hold the security configs for stream IDs
 ******************************************************************************/
const static mc_streamid_security_cfg_t tegra194_streamid_sec_cfgs[] = {
	mc_make_sec_cfg(HDAR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(HOST1XDMAR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSRD, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(SATAR, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(NVENCSWR, NON_SECURE, NO_OVERRIDE, DISABLE),
	mc_make_sec_cfg(HDAW, NON_SECURE, OVERRIDE, DISABLE),
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
	mc_make_sec_cfg(MIU0R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU0W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU1R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU1W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU2R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU2W, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU3R, NON_SECURE, OVERRIDE, DISABLE),
	mc_make_sec_cfg(MIU3W, NON_SECURE, OVERRIDE, DISABLE)
};

/* To be called by common memctrl_v2.c */
static void tegra194_memctrl_reconfig_mss_clients(void)
{
	uint32_t reg_val, wdata_0, wdata_1, wdata_2;

	wdata_0 = MC_CLIENT_HOTRESET_CTRL0_HC_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_SATA_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_VIC_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_XUSB_HOST_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_XUSB_DEV_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_TSEC_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL0_SDMMC3A_FLUSH_ENB;
	if (tegra_platform_is_silicon()) {
		wdata_0 |= MC_CLIENT_HOTRESET_CTRL0_SDMMC1A_FLUSH_ENB;
	}

	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL0, wdata_0);

	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		reg_val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS0);
	} while ((reg_val & wdata_0) != wdata_0);

	wdata_1 = MC_CLIENT_HOTRESET_CTRL1_SDMMC4A_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_SE_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_ETR_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_TSECB_FLUSH_ENB|
		  MC_CLIENT_HOTRESET_CTRL1_AXIS_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_UFSHC_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_NVDISPLAY_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_BPMP_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_AON_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_SCE_FLUSH_ENB |
		  MC_CLIENT_HOTRESET_CTRL1_VIFAL_FLUSH_ENB;
	if (tegra_platform_is_silicon()) {
		wdata_1 |= MC_CLIENT_HOTRESET_CTRL1_APE_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL1_EQOS_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL1_RCE_FLUSH_ENB;
	}
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL1, wdata_1);
	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		reg_val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS1);
	} while ((reg_val & wdata_1) != wdata_1);

	wdata_2 = MC_CLIENT_HOTRESET_CTRL2_PCIE_FLUSH_ENB |
		MC_CLIENT_HOTRESET_CTRL2_AONDMA_FLUSH_ENB |
		MC_CLIENT_HOTRESET_CTRL2_BPMPDMA_FLUSH_ENB |
		MC_CLIENT_HOTRESET_CTRL2_SCEDMA_FLUSH_ENB;
	if (tegra_platform_is_silicon()) {
		wdata_2 |= MC_CLIENT_HOTRESET_CTRL2_RCEDMA_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE5A_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE3A_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE3_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE0A_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE0A2_FLUSH_ENB |
			MC_CLIENT_HOTRESET_CTRL2_PCIE4A_FLUSH_ENB;
	}
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL2, wdata_2);
	/* Wait for HOTRESET STATUS to indicate FLUSH_DONE */
	do {
		reg_val = tegra_mc_read_32(MC_CLIENT_HOTRESET_STATUS2);
	} while ((reg_val & wdata_2) != wdata_2);

	/*
	 * Change MEMTYPE_OVERRIDE from SO_DEV -> PASSTHRU for boot and
	 * strongly ordered MSS clients.
	 *
	 * MC clients with default SO_DEV override still enabled at TSA:
	 * EQOSW, SATAW, XUSB_DEVW, XUSB_HOSTW, PCIe0w, PCIe1w, PCIe2w,
	 * PCIe3w, PCIe4w and PCIe5w.
	 */
	mc_set_tsa_w_passthrough(AONDMAW);
	mc_set_tsa_w_passthrough(AONW);
	mc_set_tsa_w_passthrough(APEDMAW);
	mc_set_tsa_w_passthrough(APEW);
	mc_set_tsa_w_passthrough(AXISW);
	mc_set_tsa_w_passthrough(BPMPDMAW);
	mc_set_tsa_w_passthrough(BPMPW);
	mc_set_tsa_w_passthrough(EQOSW);
	mc_set_tsa_w_passthrough(ETRW);
	mc_set_tsa_w_passthrough(RCEDMAW);
	mc_set_tsa_w_passthrough(RCEW);
	mc_set_tsa_w_passthrough(SCEDMAW);
	mc_set_tsa_w_passthrough(SCEW);
	mc_set_tsa_w_passthrough(SDMMCW);
	mc_set_tsa_w_passthrough(SDMMCWA);
	mc_set_tsa_w_passthrough(SDMMCWAB);
	mc_set_tsa_w_passthrough(SESWR);
	mc_set_tsa_w_passthrough(TSECSWR);
	mc_set_tsa_w_passthrough(TSECSWRB);
	mc_set_tsa_w_passthrough(UFSHCW);
	mc_set_tsa_w_passthrough(VICSWR);
	mc_set_tsa_w_passthrough(VIFALW);
	/*
	 * set HUB2 as SO_DEV_HUBID
	 */
	reg_val = tsa_read_32(PCIE0W);
	mc_set_tsa_hub2(reg_val, PCIE0W);
	reg_val = tsa_read_32(PCIE1W);
	mc_set_tsa_hub2(reg_val, PCIE1W);
	reg_val = tsa_read_32(PCIE2AW);
	mc_set_tsa_hub2(reg_val, PCIE2AW);
	reg_val = tsa_read_32(PCIE3W);
	mc_set_tsa_hub2(reg_val, PCIE3W);
	reg_val = tsa_read_32(PCIE4W);
	mc_set_tsa_hub2(reg_val, PCIE4W);
	reg_val = tsa_read_32(SATAW);
	mc_set_tsa_hub2(reg_val, SATAW);
	reg_val = tsa_read_32(XUSB_DEVW);
	mc_set_tsa_hub2(reg_val, XUSB_DEVW);
	reg_val = tsa_read_32(XUSB_HOSTW);
	mc_set_tsa_hub2(reg_val, XUSB_HOSTW);

	/*
	 * Hw Bug: 200385660, 200394107
	 * PCIE datapath hangs when there are more than 28 outstanding
	 * requests on data backbone for x1 controller. This is seen
	 * on third party PCIE IP, C1 - PCIE1W, C2 - PCIE2AW and C3 - PCIE3W.
	 *
	 * Setting Reorder depth limit, 16 which is < 28.
	 */
	mc_set_tsa_depth_limit(REORDER_DEPTH_LIMIT, PCIE1W);
	mc_set_tsa_depth_limit(REORDER_DEPTH_LIMIT, PCIE2AW);
	mc_set_tsa_depth_limit(REORDER_DEPTH_LIMIT, PCIE3W);

	/* Ordered MC Clients on Xavier are EQOS, SATA, XUSB, PCIe1 and PCIe3
	 * ISO clients(DISP, VI, EQOS) should never snoop caches and
	 *     don't need ROC/PCFIFO ordering.
	 * ISO clients(EQOS) that need ordering should use PCFIFO ordering
	 *     and bypass ROC ordering by using FORCE_NON_COHERENT path.
	 * FORCE_NON_COHERENT/FORCE_COHERENT config take precedence
	 *     over SMMU attributes.
	 * Force all Normal memory transactions from ISO and non-ISO to be
	 *     non-coherent(bypass ROC, avoid cache snoop to avoid perf hit).
	 * Force the SO_DEV transactions from ordered ISO clients(EQOS) to
	 *     non-coherent path and enable MC PCFIFO interlock for ordering.
	 * Force the SO_DEV transactions from ordered non-ISO clients (PCIe,
	 *     XUSB, SATA) to coherent so that the transactions are
	 *     ordered by ROC.
	 * PCFIFO ensure write ordering.
	 * Read after Write ordering is maintained/enforced by MC clients.
	 * Clients that need PCIe type write ordering must
	 *     go through ROC ordering.
	 * Ordering enable for Read clients is not necessary.
	 * R5's and A9 would get necessary ordering from AXI and
	 *     don't need ROC ordering enable:
	 *     - MMIO ordering is through dev mapping and MMIO
	 *       accesses bypass SMMU.
	 *     - Normal memory is accessed through SMMU and ordering is
	 *       ensured by client and AXI.
	 *     - Ack point for Normal memory is WCAM in MC.
	 *     - MMIO's can be early acked and AXI ensures dev memory ordering,
	 *       Client ensures read/write direction change ordering.
	 *     - See Bug 200312466 for more details.
	 */
	mc_set_txn_override(AONDMAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(AONDMAW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(AONR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(AONW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(APEDMAR, CGID_TAG_ADR, SO_DEV_CLIENT_AXI_ID, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(APEDMAW, CGID_TAG_ADR, SO_DEV_CLIENT_AXI_ID, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(APER, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(APEW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(AXISR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(AXISW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(BPMPDMAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(BPMPDMAW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(BPMPR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(BPMPW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(EQOSR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(EQOSW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(ETRR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ETRW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(HOST1XDMAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(MPCORER, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
	mc_set_txn_override(MPCOREW, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
	mc_set_txn_override(NVDISPLAYR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(NVDISPLAYR1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(PCIE0R, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(PCIE0R1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(PCIE0W, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(PCIE1R, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(PCIE1W, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	if (tegra_platform_is_silicon()) {
		mc_set_txn_override(PCIE2AR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE2AW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE3R, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE3W, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE4R, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE4W, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE5R, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE5W, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
		mc_set_txn_override(PCIE5R1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	}
	mc_set_txn_override(PTCR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(RCEDMAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(RCEDMAW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(RCER, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(RCEW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SATAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(SATAW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(SCEDMAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SCEDMAW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SCER, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SCEW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SDMMCR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SDMMCRAB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SDMMCRA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SDMMCW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SDMMCWA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(SDMMCWAB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	/*
	 * TO DO: make SESRD/WR FORCE_COHERENT once SE+TZ with SMMU is enabled.
	*/
	mc_set_txn_override(SESRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(SESWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(TSECSRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(TSECSRDB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(TSECSWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(TSECSWRB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(UFSHCR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(UFSHCW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(VICSRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(VICSRD1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(VICSWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(VIFALR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(VIFALW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(XUSB_DEVR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(XUSB_DEVW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(XUSB_HOSTR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(XUSB_HOSTW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT_SNOOP, FORCE_COHERENT_SNOOP);
	mc_set_txn_override(AXIAPR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(AXIAPW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA0FALRDB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA0FALWRB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA0RDA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA0RDA1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA0WRA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA1FALRDB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA1FALWRB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA1RDA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA1RDA1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(DLA1WRA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(HDAR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(HDAW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);
	mc_set_txn_override(ISPFALR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ISPFALW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ISPRA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ISPRA1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ISPWA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(ISPWB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVDEC1SRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVDEC1SRD1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVDEC1SWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVDECSRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVDECSRD1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVDECSWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVENC1SRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVENC1SRD1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVENC1SWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVENCSRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVENCSRD1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVENCSWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVJPGSRD, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(NVJPGSWR, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0RDA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0RDA1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0RDB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0RDB1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0RDC, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0WRA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0WRB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA0WRC, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1RDA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1RDA1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1RDB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1RDB1, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1RDC, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1WRA, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1WRB, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(PVA1WRC, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_COHERENT, FORCE_COHERENT);
	mc_set_txn_override(VIW, CGID_TAG_ADR, SO_DEV_ZERO, FORCE_NON_COHERENT, FORCE_NON_COHERENT);

	if (tegra_platform_is_silicon()) {
		mc_set_txn_override(MIU0R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU0W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU1R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU1W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU2R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU2W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU3R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU3W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU4R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU4W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU5R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU5W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU6R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU6W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU7R, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
		mc_set_txn_override(MIU7W, CGID_TAG_ADR, SO_DEV_ZERO, NO_OVERRIDE, NO_OVERRIDE);
	}

	/*
	 * At this point, ordering can occur at SCF. So, remove PCFIFO's
	 * control over ordering requests.
	 *
	 * Change PCFIFO_*_ORDERED_CLIENT from ORDERED -> UNORDERED for
	 * boot and strongly ordered MSS clients
	 */
	reg_val = MC_PCFIFO_CLIENT_CONFIG2_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(2, XUSB_HOSTW) &
		mc_set_pcfifo_unordered_boot_so_mss(2, TSECSWR);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG2, reg_val);

	reg_val = MC_PCFIFO_CLIENT_CONFIG3_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(3, SDMMCWA) &
		mc_set_pcfifo_unordered_boot_so_mss(3, SDMMCW) &
		mc_set_pcfifo_unordered_boot_so_mss(3, SDMMCWAB) &
		mc_set_pcfifo_unordered_boot_so_mss(3, VICSWR) &
		mc_set_pcfifo_unordered_boot_so_mss(3, APEW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG3, reg_val);

	reg_val = MC_PCFIFO_CLIENT_CONFIG4_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(4, SESWR) &
		mc_set_pcfifo_unordered_boot_so_mss(4, ETRW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, TSECSWRB) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AXISW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, UFSHCW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, BPMPW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, BPMPDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AONW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, AONDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, SCEW) &
		mc_set_pcfifo_unordered_boot_so_mss(4, SCEDMAW);
	/* EQOSW has PCFIFO order enabled. */
	reg_val |= mc_set_pcfifo_unordered_boot_so_mss(4, EQOSW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG4, reg_val);

	reg_val = MC_PCFIFO_CLIENT_CONFIG5_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(5, APEDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(5, VIFALW);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG5, reg_val);

	reg_val = MC_PCFIFO_CLIENT_CONFIG6_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(6, RCEW) &
		mc_set_pcfifo_unordered_boot_so_mss(6, RCEDMAW) &
		mc_set_pcfifo_unordered_boot_so_mss(6, PCIE0W);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG6, reg_val);

	reg_val = MC_PCFIFO_CLIENT_CONFIG7_RESET_VAL &
		mc_set_pcfifo_unordered_boot_so_mss(7, PCIE4W) &
		mc_set_pcfifo_unordered_boot_so_mss(7, PCIE5W);
	tegra_mc_write_32(MC_PCFIFO_CLIENT_CONFIG7, reg_val);

	/* Set Order Id only for the clients having non zero order id */
	reg_val = mc_client_order_id(MC_CLIENT_ORDER_ID_9_RESET_VAL, 9, XUSB_HOSTW);
	tegra_mc_write_32(MC_CLIENT_ORDER_ID_9, reg_val);

	reg_val = mc_client_order_id(MC_CLIENT_ORDER_ID_27_RESET_VAL, 27, PCIE0W);
	tegra_mc_write_32(MC_CLIENT_ORDER_ID_27, reg_val);

	reg_val = mc_client_order_id(MC_CLIENT_ORDER_ID_28_RESET_VAL, 28, PCIE4W);
	reg_val = mc_client_order_id(reg_val, 28, PCIE5W);
	tegra_mc_write_32(MC_CLIENT_ORDER_ID_28, reg_val);

	/*
	 * Set VC Id only for the clients having different reset values like
	 * SDMMCRAB, SDMMCWAB, SESRD, SESWR, TSECSRD,TSECSRDB, TSECSWR and
	 * TSECSWRB clients
	 */
	reg_val = mc_hub_vc_id(MC_HUB_PC_VC_ID_0_RESET_VAL, 0, APB);
	tegra_mc_write_32(MC_HUB_PC_VC_ID_0, reg_val);

	/* SDMMCRAB and SDMMCWAB clients */
	reg_val = mc_hub_vc_id(MC_HUB_PC_VC_ID_2_RESET_VAL, 2, SD);
	tegra_mc_write_32(MC_HUB_PC_VC_ID_2, reg_val);

	reg_val = mc_hub_vc_id(MC_HUB_PC_VC_ID_4_RESET_VAL, 4, NIC);
	tegra_mc_write_32(MC_HUB_PC_VC_ID_4, reg_val);

	reg_val = mc_hub_vc_id(MC_HUB_PC_VC_ID_12_RESET_VAL, 12, UFSHCPC2);
	tegra_mc_write_32(MC_HUB_PC_VC_ID_12, reg_val);

	wdata_0 = MC_CLIENT_HOTRESET_CTRL0_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL0, wdata_0);

	wdata_1 = MC_CLIENT_HOTRESET_CTRL1_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL1, wdata_1);

	wdata_2 = MC_CLIENT_HOTRESET_CTRL2_RESET_VAL;
	tegra_mc_write_32(MC_CLIENT_HOTRESET_CTRL2, wdata_2);

	reg_val = MC_COALESCE_CTRL_COALESCER_ENABLE;
	tegra_mc_write_32(MC_COALESCE_CTRL, reg_val);

	/*
	 * WAR to hardware bug 1953865: Coalescer must be disabled
	 * for PVA0RDC and PVA1RDC interfaces.
	 */
	reg_val = tegra_mc_read_32(MC_COALESCE_CONFIG_6_0);
	reg_val &= ~(MC_COALESCE_CONFIG_6_0_PVA0RDC_COALESCER_ENABLED |
		     MC_COALESCE_CONFIG_6_0_PVA1RDC_COALESCER_ENABLED);
	tegra_mc_write_32(MC_COALESCE_CONFIG_6_0, reg_val);
}

/*******************************************************************************
 * Struct to hold the memory controller settings
 ******************************************************************************/
static tegra_mc_settings_t tegra194_mc_settings = {
	.streamid_override_cfg = tegra194_streamid_override_regs,
	.num_streamid_override_cfgs = (uint32_t)ARRAY_SIZE(tegra194_streamid_override_regs),
	.streamid_security_cfg = tegra194_streamid_sec_cfgs,
	.num_streamid_security_cfgs = (uint32_t)ARRAY_SIZE(tegra194_streamid_sec_cfgs),
	.reconfig_mss_clients = tegra194_memctrl_reconfig_mss_clients
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

		tegra_mc_write_32(MC_SECURITY_CFG0_0, (uint32_t)phys_base);
		tegra_mc_write_32(MC_SECURITY_CFG3_0, (uint32_t)(phys_base >> 32));
		tegra_mc_write_32(MC_SECURITY_CFG1_0, (uint32_t)(size_in_bytes >> 20));

		/*
		 * MCE propagates the security configuration values across the
		 * CCPLEX.
		 */
		(void)mce_update_gsc_tzdram();
	}
}
