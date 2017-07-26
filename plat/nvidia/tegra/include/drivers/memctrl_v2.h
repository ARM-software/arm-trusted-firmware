/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMCTRL_V2_H
#define MEMCTRL_V2_H

#include <tegra_def.h>

#ifndef __ASSEMBLY__

#include <stdint.h>

/*******************************************************************************
 * StreamID to indicate no SMMU translations (requests to be steered on the
 * SMMU bypass path)
 ******************************************************************************/
#define MC_STREAM_ID_MAX			0x7FU

/*******************************************************************************
 * Stream ID Override Config registers
 ******************************************************************************/
#define MC_STREAMID_OVERRIDE_CFG_PTCR		0x000U
#define MC_STREAMID_OVERRIDE_CFG_AFIR		0x070U
#define MC_STREAMID_OVERRIDE_CFG_HDAR		0x0A8U
#define MC_STREAMID_OVERRIDE_CFG_HOST1XDMAR	0x0B0U
#define MC_STREAMID_OVERRIDE_CFG_NVENCSRD	0x0E0U
#define MC_STREAMID_OVERRIDE_CFG_SATAR		0x0F8U
#define MC_STREAMID_OVERRIDE_CFG_MPCORER	0x138U
#define MC_STREAMID_OVERRIDE_CFG_NVENCSWR	0x158U
#define MC_STREAMID_OVERRIDE_CFG_AFIW		0x188U
#define MC_STREAMID_OVERRIDE_CFG_HDAW		0x1A8U
#define MC_STREAMID_OVERRIDE_CFG_MPCOREW	0x1C8U
#define MC_STREAMID_OVERRIDE_CFG_SATAW		0x1E8U
#define MC_STREAMID_OVERRIDE_CFG_ISPRA		0x220U
#define MC_STREAMID_OVERRIDE_CFG_ISPWA		0x230U
#define MC_STREAMID_OVERRIDE_CFG_ISPWB		0x238U
#define MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTR	0x250U
#define MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTW	0x258U
#define MC_STREAMID_OVERRIDE_CFG_XUSB_DEVR	0x260U
#define MC_STREAMID_OVERRIDE_CFG_XUSB_DEVW	0x268U
#define MC_STREAMID_OVERRIDE_CFG_TSECSRD	0x2A0U
#define MC_STREAMID_OVERRIDE_CFG_TSECSWR	0x2A8U
#define MC_STREAMID_OVERRIDE_CFG_GPUSRD		0x2C0U
#define MC_STREAMID_OVERRIDE_CFG_GPUSWR		0x2C8U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRA	0x300U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRAA	0x308U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCR		0x310U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRAB	0x318U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWA	0x320U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWAA	0x328U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCW		0x330U
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWAB	0x338U
#define MC_STREAMID_OVERRIDE_CFG_VICSRD		0x360U
#define MC_STREAMID_OVERRIDE_CFG_VICSWR		0x368U
#define MC_STREAMID_OVERRIDE_CFG_VIW		0x390U
#define MC_STREAMID_OVERRIDE_CFG_NVDECSRD	0x3C0U
#define MC_STREAMID_OVERRIDE_CFG_NVDECSWR	0x3C8U
#define MC_STREAMID_OVERRIDE_CFG_APER		0x3D0U
#define MC_STREAMID_OVERRIDE_CFG_APEW		0x3D8U
#define MC_STREAMID_OVERRIDE_CFG_NVJPGSRD	0x3F0U
#define MC_STREAMID_OVERRIDE_CFG_NVJPGSWR	0x3F8U
#define MC_STREAMID_OVERRIDE_CFG_SESRD		0x400U
#define MC_STREAMID_OVERRIDE_CFG_SESWR		0x408U
#define MC_STREAMID_OVERRIDE_CFG_ETRR		0x420U
#define MC_STREAMID_OVERRIDE_CFG_ETRW		0x428U
#define MC_STREAMID_OVERRIDE_CFG_TSECSRDB	0x430U
#define MC_STREAMID_OVERRIDE_CFG_TSECSWRB	0x438U
#define MC_STREAMID_OVERRIDE_CFG_GPUSRD2	0x440U
#define MC_STREAMID_OVERRIDE_CFG_GPUSWR2	0x448U
#define MC_STREAMID_OVERRIDE_CFG_AXISR		0x460U
#define MC_STREAMID_OVERRIDE_CFG_AXISW		0x468U
#define MC_STREAMID_OVERRIDE_CFG_EQOSR		0x470U
#define MC_STREAMID_OVERRIDE_CFG_EQOSW		0x478U
#define MC_STREAMID_OVERRIDE_CFG_UFSHCR		0x480U
#define MC_STREAMID_OVERRIDE_CFG_UFSHCW		0x488U
#define MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR	0x490U
#define MC_STREAMID_OVERRIDE_CFG_BPMPR		0x498U
#define MC_STREAMID_OVERRIDE_CFG_BPMPW		0x4A0U
#define MC_STREAMID_OVERRIDE_CFG_BPMPDMAR	0x4A8U
#define MC_STREAMID_OVERRIDE_CFG_BPMPDMAW	0x4B0U
#define MC_STREAMID_OVERRIDE_CFG_AONR		0x4B8U
#define MC_STREAMID_OVERRIDE_CFG_AONW		0x4C0U
#define MC_STREAMID_OVERRIDE_CFG_AONDMAR	0x4C8U
#define MC_STREAMID_OVERRIDE_CFG_AONDMAW	0x4D0U
#define MC_STREAMID_OVERRIDE_CFG_SCER		0x4D8U
#define MC_STREAMID_OVERRIDE_CFG_SCEW		0x4E0U
#define MC_STREAMID_OVERRIDE_CFG_SCEDMAR	0x4E8U
#define MC_STREAMID_OVERRIDE_CFG_SCEDMAW	0x4F0U
#define MC_STREAMID_OVERRIDE_CFG_APEDMAR	0x4F8U
#define MC_STREAMID_OVERRIDE_CFG_APEDMAW	0x500U
#define MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR1	0x508U
#define MC_STREAMID_OVERRIDE_CFG_VICSRD1	0x510U
#define MC_STREAMID_OVERRIDE_CFG_NVDECSRD1	0x518U

/*******************************************************************************
 * Macro to calculate Security cfg register addr from StreamID Override register
 ******************************************************************************/
#define MC_STREAMID_OVERRIDE_TO_SECURITY_CFG(addr) ((addr) + (uint32_t)sizeof(uint32_t))

#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_NO_OVERRIDE_SO_DEV		(0U << 4)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_FORCE_NON_COHERENT_SO_DEV	(1U << 4)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_FORCE_COHERENT_SO_DEV		(2U << 4)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_FORCE_COHERENT_SNOOP_SO_DEV	(3U << 4)

#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_NO_OVERRIDE_NORMAL		(0U << 8)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_FORCE_NON_COHERENT_NORMAL	(1U << 8)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_FORCE_COHERENT_NORMAL		(2U << 8)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_FORCE_COHERENT_SNOOP_NORMAL	(3U << 8)

#define MC_TXN_OVERRIDE_CONFIG_CGID_SO_DEV_ZERO				(0U << 12)
#define MC_TXN_OVERRIDE_CONFIG_CGID_SO_DEV_CLIENT_AXI_ID		(1U << 12)

/*******************************************************************************
 * Memory Controller transaction override config registers
 ******************************************************************************/
#define MC_TXN_OVERRIDE_CONFIG_HDAR		0x10a8U
#define MC_TXN_OVERRIDE_CONFIG_BPMPW		0x14a0U
#define MC_TXN_OVERRIDE_CONFIG_PTCR		0x1000U
#define MC_TXN_OVERRIDE_CONFIG_NVDISPLAYR	0x1490U
#define MC_TXN_OVERRIDE_CONFIG_EQOSW		0x1478U
#define MC_TXN_OVERRIDE_CONFIG_NVJPGSWR		0x13f8U
#define MC_TXN_OVERRIDE_CONFIG_ISPRA		0x1220U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCWAA		0x1328U
#define MC_TXN_OVERRIDE_CONFIG_VICSRD		0x1360U
#define MC_TXN_OVERRIDE_CONFIG_MPCOREW		0x11c8U
#define MC_TXN_OVERRIDE_CONFIG_GPUSRD		0x12c0U
#define MC_TXN_OVERRIDE_CONFIG_AXISR		0x1460U
#define MC_TXN_OVERRIDE_CONFIG_SCEDMAW		0x14f0U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCW		0x1330U
#define MC_TXN_OVERRIDE_CONFIG_EQOSR		0x1470U
#define MC_TXN_OVERRIDE_CONFIG_APEDMAR		0x14f8U
#define MC_TXN_OVERRIDE_CONFIG_NVENCSRD		0x10e0U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCRAB		0x1318U
#define MC_TXN_OVERRIDE_CONFIG_VICSRD1		0x1510U
#define MC_TXN_OVERRIDE_CONFIG_BPMPDMAR		0x14a8U
#define MC_TXN_OVERRIDE_CONFIG_VIW		0x1390U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCRAA		0x1308U
#define MC_TXN_OVERRIDE_CONFIG_AXISW		0x1468U
#define MC_TXN_OVERRIDE_CONFIG_XUSB_DEVR	0x1260U
#define MC_TXN_OVERRIDE_CONFIG_UFSHCR		0x1480U
#define MC_TXN_OVERRIDE_CONFIG_TSECSWR		0x12a8U
#define MC_TXN_OVERRIDE_CONFIG_GPUSWR		0x12c8U
#define MC_TXN_OVERRIDE_CONFIG_SATAR		0x10f8U
#define MC_TXN_OVERRIDE_CONFIG_XUSB_HOSTW	0x1258U
#define MC_TXN_OVERRIDE_CONFIG_TSECSWRB		0x1438U
#define MC_TXN_OVERRIDE_CONFIG_GPUSRD2		0x1440U
#define MC_TXN_OVERRIDE_CONFIG_SCEDMAR		0x14e8U
#define MC_TXN_OVERRIDE_CONFIG_GPUSWR2		0x1448U
#define MC_TXN_OVERRIDE_CONFIG_AONDMAW		0x14d0U
#define MC_TXN_OVERRIDE_CONFIG_APEDMAW		0x1500U
#define MC_TXN_OVERRIDE_CONFIG_AONW		0x14c0U
#define MC_TXN_OVERRIDE_CONFIG_HOST1XDMAR	0x10b0U
#define MC_TXN_OVERRIDE_CONFIG_ETRR		0x1420U
#define MC_TXN_OVERRIDE_CONFIG_SESWR		0x1408U
#define MC_TXN_OVERRIDE_CONFIG_NVJPGSRD		0x13f0U
#define MC_TXN_OVERRIDE_CONFIG_NVDECSRD		0x13c0U
#define MC_TXN_OVERRIDE_CONFIG_TSECSRDB		0x1430U
#define MC_TXN_OVERRIDE_CONFIG_BPMPDMAW		0x14b0U
#define MC_TXN_OVERRIDE_CONFIG_APER		0x13d0U
#define MC_TXN_OVERRIDE_CONFIG_NVDECSRD1	0x1518U
#define MC_TXN_OVERRIDE_CONFIG_XUSB_HOSTR	0x1250U
#define MC_TXN_OVERRIDE_CONFIG_ISPWA		0x1230U
#define MC_TXN_OVERRIDE_CONFIG_SESRD		0x1400U
#define MC_TXN_OVERRIDE_CONFIG_SCER		0x14d8U
#define MC_TXN_OVERRIDE_CONFIG_AONR		0x14b8U
#define MC_TXN_OVERRIDE_CONFIG_MPCORER		0x1138U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCWA		0x1320U
#define MC_TXN_OVERRIDE_CONFIG_HDAW		0x11a8U
#define MC_TXN_OVERRIDE_CONFIG_NVDECSWR		0x13c8U
#define MC_TXN_OVERRIDE_CONFIG_UFSHCW		0x1488U
#define MC_TXN_OVERRIDE_CONFIG_AONDMAR		0x14c8U
#define MC_TXN_OVERRIDE_CONFIG_SATAW		0x11e8U
#define MC_TXN_OVERRIDE_CONFIG_ETRW		0x1428U
#define MC_TXN_OVERRIDE_CONFIG_VICSWR		0x1368U
#define MC_TXN_OVERRIDE_CONFIG_NVENCSWR		0x1158U
#define MC_TXN_OVERRIDE_CONFIG_AFIR		0x1070U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCWAB		0x1338U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCRA		0x1300U
#define MC_TXN_OVERRIDE_CONFIG_NVDISPLAYR1	0x1508U
#define MC_TXN_OVERRIDE_CONFIG_ISPWB		0x1238U
#define MC_TXN_OVERRIDE_CONFIG_BPMPR		0x1498U
#define MC_TXN_OVERRIDE_CONFIG_APEW		0x13d8U
#define MC_TXN_OVERRIDE_CONFIG_SDMMCR		0x1310U
#define MC_TXN_OVERRIDE_CONFIG_XUSB_DEVW	0x1268U
#define MC_TXN_OVERRIDE_CONFIG_TSECSRD		0x12a0U
#define MC_TXN_OVERRIDE_CONFIG_AFIW		0x1188U
#define MC_TXN_OVERRIDE_CONFIG_SCEW		0x14e0U

/*******************************************************************************
 * Structure to hold the transaction override settings to use to override
 * client inputs
 ******************************************************************************/
typedef struct mc_txn_override_cfg {
	uint32_t offset;
	uint8_t cgid_tag;
} mc_txn_override_cfg_t;

#define mc_make_txn_override_cfg(off, val) \
	{ \
		.offset = MC_TXN_OVERRIDE_CONFIG_ ## off, \
		.cgid_tag = MC_TXN_OVERRIDE_ ## val \
	}

/*******************************************************************************
 * Structure to hold the Stream ID to use to override client inputs
 ******************************************************************************/
typedef struct mc_streamid_override_cfg {
	uint32_t offset;
	uint8_t stream_id;
} mc_streamid_override_cfg_t;

/*******************************************************************************
 * Structure to hold the Stream ID Security Configuration settings
 ******************************************************************************/
typedef struct mc_streamid_security_cfg {
	char *name;
	uint32_t offset;
	int override_enable;
	int override_client_inputs;
	int override_client_ns_flag;
} mc_streamid_security_cfg_t;

#define OVERRIDE_DISABLE				1U
#define OVERRIDE_ENABLE					0U
#define CLIENT_FLAG_SECURE				0U
#define CLIENT_FLAG_NON_SECURE				1U
#define CLIENT_INPUTS_OVERRIDE				1U
#define CLIENT_INPUTS_NO_OVERRIDE			0U

#define mc_make_sec_cfg(off, ns, ovrrd, access) \
	{ \
		.name = # off, \
		.offset = MC_STREAMID_OVERRIDE_TO_SECURITY_CFG( \
				MC_STREAMID_OVERRIDE_CFG_ ## off), \
		.override_client_ns_flag = CLIENT_FLAG_ ## ns, \
		.override_client_inputs = CLIENT_INPUTS_ ## ovrrd, \
		.override_enable = OVERRIDE_ ## access \
	}

/*******************************************************************************
 * Structure to hold Memory Controller's Configuration settings
 ******************************************************************************/
typedef struct tegra_mc_settings {
	const uint32_t *streamid_override_cfg;
	uint32_t num_streamid_override_cfgs;
	const mc_streamid_security_cfg_t *streamid_security_cfg;
	uint32_t num_streamid_security_cfgs;
	const mc_txn_override_cfg_t *txn_override_cfg;
	uint32_t num_txn_override_cfgs;
} tegra_mc_settings_t;

#endif /* __ASSEMBLY__ */

/*******************************************************************************
 * Memory Controller SMMU Bypass config register
 ******************************************************************************/
#define MC_SMMU_BYPASS_CONFIG			0x1820U
#define MC_SMMU_BYPASS_CTRL_MASK		0x3U
#define MC_SMMU_BYPASS_CTRL_SHIFT		0U
#define MC_SMMU_CTRL_TBU_BYPASS_ALL		(0U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_RSVD			(1U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID	(2U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_NONE		(3U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT	(1U << 31)
#define MC_SMMU_BYPASS_CONFIG_SETTINGS		(MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT | \
						 MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID)

#define MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_CGID	(1U << 0)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_OVERRIDE_SO_DEV	(2U << 4)
#define MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_SO_DEV_CGID_SO_DEV_CLIENT	(1U << 12)

/*******************************************************************************
 * Non-SO_DEV transactions override values for CGID_TAG bitfield for the
 * MC_TXN_OVERRIDE_CONFIG_{module} registers
 ******************************************************************************/
#define MC_TXN_OVERRIDE_CGID_TAG_DEFAULT	0U
#define MC_TXN_OVERRIDE_CGID_TAG_CLIENT_AXI_ID	1U
#define MC_TXN_OVERRIDE_CGID_TAG_ZERO		2U
#define MC_TXN_OVERRIDE_CGID_TAG_ADR		3U
#define MC_TXN_OVERRIDE_CGID_TAG_MASK		3U

/*******************************************************************************
 * Memory Controller Reset Control registers
 ******************************************************************************/
#define MC_CLIENT_HOTRESET_CTRL0			0x200U
#define  MC_CLIENT_HOTRESET_CTRL0_RESET_VAL		0U
#define  MC_CLIENT_HOTRESET_CTRL0_AFI_FLUSH_ENB		(1U << 0)
#define  MC_CLIENT_HOTRESET_CTRL0_HC_FLUSH_ENB		(1U << 6)
#define  MC_CLIENT_HOTRESET_CTRL0_HDA_FLUSH_ENB		(1U << 7)
#define  MC_CLIENT_HOTRESET_CTRL0_ISP2_FLUSH_ENB	(1U << 8)
#define  MC_CLIENT_HOTRESET_CTRL0_MPCORE_FLUSH_ENB	(1U << 9)
#define  MC_CLIENT_HOTRESET_CTRL0_NVENC_FLUSH_ENB	(1U << 11)
#define  MC_CLIENT_HOTRESET_CTRL0_SATA_FLUSH_ENB	(1U << 15)
#define  MC_CLIENT_HOTRESET_CTRL0_VI_FLUSH_ENB		(1U << 17)
#define  MC_CLIENT_HOTRESET_CTRL0_VIC_FLUSH_ENB		(1U << 18)
#define  MC_CLIENT_HOTRESET_CTRL0_XUSB_HOST_FLUSH_ENB	(1U << 19)
#define  MC_CLIENT_HOTRESET_CTRL0_XUSB_DEV_FLUSH_ENB	(1U << 20)
#define  MC_CLIENT_HOTRESET_CTRL0_TSEC_FLUSH_ENB	(1U << 22)
#define  MC_CLIENT_HOTRESET_CTRL0_SDMMC1A_FLUSH_ENB	(1U << 29)
#define  MC_CLIENT_HOTRESET_CTRL0_SDMMC2A_FLUSH_ENB	(1U << 30)
#define  MC_CLIENT_HOTRESET_CTRL0_SDMMC3A_FLUSH_ENB	(1U << 31)
#define MC_CLIENT_HOTRESET_STATUS0			0x204U
#define MC_CLIENT_HOTRESET_CTRL1			0x970U
#define  MC_CLIENT_HOTRESET_CTRL1_RESET_VAL		0U
#define  MC_CLIENT_HOTRESET_CTRL1_SDMMC4A_FLUSH_ENB	(1U << 0)
#define  MC_CLIENT_HOTRESET_CTRL1_GPU_FLUSH_ENB		(1U << 2)
#define  MC_CLIENT_HOTRESET_CTRL1_NVDEC_FLUSH_ENB	(1U << 5)
#define  MC_CLIENT_HOTRESET_CTRL1_APE_FLUSH_ENB		(1U << 6)
#define  MC_CLIENT_HOTRESET_CTRL1_SE_FLUSH_ENB		(1U << 7)
#define  MC_CLIENT_HOTRESET_CTRL1_NVJPG_FLUSH_ENB	(1U << 8)
#define  MC_CLIENT_HOTRESET_CTRL1_ETR_FLUSH_ENB		(1U << 12)
#define  MC_CLIENT_HOTRESET_CTRL1_TSECB_FLUSH_ENB	(1U << 13)
#define  MC_CLIENT_HOTRESET_CTRL1_AXIS_FLUSH_ENB	(1U << 18)
#define  MC_CLIENT_HOTRESET_CTRL1_EQOS_FLUSH_ENB	(1U << 19)
#define  MC_CLIENT_HOTRESET_CTRL1_UFSHC_FLUSH_ENB	(1U << 20)
#define  MC_CLIENT_HOTRESET_CTRL1_NVDISPLAY_FLUSH_ENB	(1U << 21)
#define  MC_CLIENT_HOTRESET_CTRL1_BPMP_FLUSH_ENB	(1U << 22)
#define  MC_CLIENT_HOTRESET_CTRL1_AON_FLUSH_ENB		(1U << 23)
#define  MC_CLIENT_HOTRESET_CTRL1_SCE_FLUSH_ENB		(1U << 24)
#define MC_CLIENT_HOTRESET_STATUS1			0x974U

/*******************************************************************************
 * Memory Controller's PCFIFO client configuration registers
 ******************************************************************************/
#define MC_PCFIFO_CLIENT_CONFIG1				0xdd4UL
#define  MC_PCFIFO_CLIENT_CONFIG1_RESET_VAL			0x20000UL
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_AFIW_UNORDERED		(0UL << 17)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_AFIW_MASK		(1UL << 17)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_HDAW_UNORDERED		(0UL << 21)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_HDAW_MASK		(1UL << 21)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_SATAW_UNORDERED	(0UL << 29)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_SATAW_MASK		(1UL << 29)

#define MC_PCFIFO_CLIENT_CONFIG2				0xdd8UL
#define  MC_PCFIFO_CLIENT_CONFIG2_RESET_VAL			0x20000UL
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_HOSTW_UNORDERED	(0UL << 11)
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_HOSTW_MASK	(1UL << 11)
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_DEVW_UNORDERED	(0UL << 13)
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_DEVW_MASK		(1UL << 13)

#define MC_PCFIFO_CLIENT_CONFIG3				0xddcUL
#define  MC_PCFIFO_CLIENT_CONFIG3_RESET_VAL			0UL
#define  MC_PCFIFO_CLIENT_CONFIG3_PCFIFO_SDMMCWAB_UNORDERED	(0UL << 7)
#define  MC_PCFIFO_CLIENT_CONFIG3_PCFIFO_SDMMCWAB_MASK		(1UL << 7)

#define MC_PCFIFO_CLIENT_CONFIG4				0xde0UL
#define  MC_PCFIFO_CLIENT_CONFIG4_RESET_VAL			0UL
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SESWR_UNORDERED	(0UL << 1)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SESWR_MASK		(1UL << 1)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_ETRW_UNORDERED		(0UL << 5)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_ETRW_MASK		(1UL << 5)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AXISW_UNORDERED	(0UL << 13)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AXISW_MASK		(1UL << 13)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_EQOSW_UNORDERED	(0UL << 15)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_EQOSW_ORDERED		(1UL << 15)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_EQOSW_MASK		(1UL << 15)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_UFSHCW_UNORDERED	(0UL << 17)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_UFSHCW_MASK		(1UL << 17)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_BPMPDMAW_UNORDERED	(0UL << 22)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_BPMPDMAW_MASK		(1UL << 22)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AONDMAW_UNORDERED	(0UL << 26)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AONDMAW_MASK		(1UL << 26)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SCEDMAW_UNORDERED	(0UL << 30)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SCEDMAW_MASK		(1UL << 30)

#define MC_PCFIFO_CLIENT_CONFIG5				0xbf4UL
#define  MC_PCFIFO_CLIENT_CONFIG5_RESET_VAL			0UL
#define  MC_PCFIFO_CLIENT_CONFIG5_PCFIFO_APEDMAW_UNORDERED	(0UL << 0)
#define  MC_PCFIFO_CLIENT_CONFIG5_PCFIFO_APEDMAW_MASK		(1UL << 0)

#ifndef __ASSEMBLY__

#include <lib/mmio.h>

static inline uint32_t tegra_mc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_BASE + off);
}

static inline void tegra_mc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_BASE + off, val);
}

static inline uint32_t tegra_mc_streamid_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_STREAMID_BASE + off);
}

static inline void tegra_mc_streamid_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_STREAMID_BASE + off, val);
}

#define mc_set_pcfifo_unordered_boot_so_mss(id, client) \
	((uint32_t)~MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_MASK | \
	 MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_UNORDERED)

#define mc_set_pcfifo_ordered_boot_so_mss(id, client) \
	 MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_ORDERED

#define mc_set_tsa_passthrough(client) \
	{ \
		mmio_write_32(TEGRA_TSA_BASE + TSA_CONFIG_STATIC0_CSW_##client, \
			(TSA_CONFIG_STATIC0_CSW_##client##_RESET & \
			 (uint32_t)~TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK) | \
			(uint32_t)TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU); \
	}

#define mc_set_txn_override(client, normal_axi_id, so_dev_axi_id, normal_override, so_dev_override) \
	{ \
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_##client, \
				  MC_TXN_OVERRIDE_##normal_axi_id | \
				  MC_TXN_OVERRIDE_CONFIG_COH_PATH_##so_dev_override##_SO_DEV | \
				  MC_TXN_OVERRIDE_CONFIG_COH_PATH_##normal_override##_NORMAL | \
				  MC_TXN_OVERRIDE_CONFIG_CGID_##so_dev_axi_id); \
	}

/*******************************************************************************
 * Handler to read memory configuration settings
 *
 * Implemented by SoCs under tegra/soc/txxx
 ******************************************************************************/
tegra_mc_settings_t *tegra_get_mc_settings(void);

#endif /* __ASSMEBLY__ */

#endif /* MEMCTRL_V2_H */
