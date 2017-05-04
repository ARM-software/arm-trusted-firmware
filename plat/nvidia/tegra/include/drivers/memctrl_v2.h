/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MEMCTRLV2_H__
#define __MEMCTRLV2_H__

#include <tegra_def.h>

#ifndef __ASSEMBLY__

#include <sys/types.h>

/*******************************************************************************
 * StreamID to indicate no SMMU translations (requests to be steered on the
 * SMMU bypass path)
 ******************************************************************************/
#define MC_STREAM_ID_MAX			0x7F

/*******************************************************************************
 * Stream ID Override Config registers
 ******************************************************************************/
#define MC_STREAMID_OVERRIDE_CFG_PTCR		0x000
#define MC_STREAMID_OVERRIDE_CFG_AFIR		0x070
#define MC_STREAMID_OVERRIDE_CFG_HDAR		0x0A8
#define MC_STREAMID_OVERRIDE_CFG_HOST1XDMAR	0x0B0
#define MC_STREAMID_OVERRIDE_CFG_NVENCSRD	0x0E0
#define MC_STREAMID_OVERRIDE_CFG_SATAR		0x0F8
#define MC_STREAMID_OVERRIDE_CFG_MPCORER	0x138
#define MC_STREAMID_OVERRIDE_CFG_NVENCSWR	0x158
#define MC_STREAMID_OVERRIDE_CFG_AFIW		0x188
#define MC_STREAMID_OVERRIDE_CFG_HDAW		0x1A8
#define MC_STREAMID_OVERRIDE_CFG_MPCOREW	0x1C8
#define MC_STREAMID_OVERRIDE_CFG_SATAW		0x1E8
#define MC_STREAMID_OVERRIDE_CFG_ISPRA		0x220
#define MC_STREAMID_OVERRIDE_CFG_ISPWA		0x230
#define MC_STREAMID_OVERRIDE_CFG_ISPWB		0x238
#define MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTR	0x250
#define MC_STREAMID_OVERRIDE_CFG_XUSB_HOSTW	0x258
#define MC_STREAMID_OVERRIDE_CFG_XUSB_DEVR	0x260
#define MC_STREAMID_OVERRIDE_CFG_XUSB_DEVW	0x268
#define MC_STREAMID_OVERRIDE_CFG_TSECSRD	0x2A0
#define MC_STREAMID_OVERRIDE_CFG_TSECSWR	0x2A8
#define MC_STREAMID_OVERRIDE_CFG_GPUSRD		0x2C0
#define MC_STREAMID_OVERRIDE_CFG_GPUSWR		0x2C8
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRA	0x300
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRAA	0x308
#define MC_STREAMID_OVERRIDE_CFG_SDMMCR		0x310
#define MC_STREAMID_OVERRIDE_CFG_SDMMCRAB	0x318
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWA	0x320
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWAA	0x328
#define MC_STREAMID_OVERRIDE_CFG_SDMMCW		0x330
#define MC_STREAMID_OVERRIDE_CFG_SDMMCWAB	0x338
#define MC_STREAMID_OVERRIDE_CFG_VICSRD		0x360
#define MC_STREAMID_OVERRIDE_CFG_VICSWR		0x368
#define MC_STREAMID_OVERRIDE_CFG_VIW		0x390
#define MC_STREAMID_OVERRIDE_CFG_NVDECSRD	0x3C0
#define MC_STREAMID_OVERRIDE_CFG_NVDECSWR	0x3C8
#define MC_STREAMID_OVERRIDE_CFG_APER		0x3D0
#define MC_STREAMID_OVERRIDE_CFG_APEW		0x3D8
#define MC_STREAMID_OVERRIDE_CFG_NVJPGSRD	0x3F0
#define MC_STREAMID_OVERRIDE_CFG_NVJPGSWR	0x3F8
#define MC_STREAMID_OVERRIDE_CFG_SESRD		0x400
#define MC_STREAMID_OVERRIDE_CFG_SESWR		0x408
#define MC_STREAMID_OVERRIDE_CFG_ETRR		0x420
#define MC_STREAMID_OVERRIDE_CFG_ETRW		0x428
#define MC_STREAMID_OVERRIDE_CFG_TSECSRDB	0x430
#define MC_STREAMID_OVERRIDE_CFG_TSECSWRB	0x438
#define MC_STREAMID_OVERRIDE_CFG_GPUSRD2	0x440
#define MC_STREAMID_OVERRIDE_CFG_GPUSWR2	0x448
#define MC_STREAMID_OVERRIDE_CFG_AXISR		0x460
#define MC_STREAMID_OVERRIDE_CFG_AXISW		0x468
#define MC_STREAMID_OVERRIDE_CFG_EQOSR		0x470
#define MC_STREAMID_OVERRIDE_CFG_EQOSW		0x478
#define MC_STREAMID_OVERRIDE_CFG_UFSHCR		0x480
#define MC_STREAMID_OVERRIDE_CFG_UFSHCW		0x488
#define MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR	0x490
#define MC_STREAMID_OVERRIDE_CFG_BPMPR		0x498
#define MC_STREAMID_OVERRIDE_CFG_BPMPW		0x4A0
#define MC_STREAMID_OVERRIDE_CFG_BPMPDMAR	0x4A8
#define MC_STREAMID_OVERRIDE_CFG_BPMPDMAW	0x4B0
#define MC_STREAMID_OVERRIDE_CFG_AONR		0x4B8
#define MC_STREAMID_OVERRIDE_CFG_AONW		0x4C0
#define MC_STREAMID_OVERRIDE_CFG_AONDMAR	0x4C8
#define MC_STREAMID_OVERRIDE_CFG_AONDMAW	0x4D0
#define MC_STREAMID_OVERRIDE_CFG_SCER		0x4D8
#define MC_STREAMID_OVERRIDE_CFG_SCEW		0x4E0
#define MC_STREAMID_OVERRIDE_CFG_SCEDMAR	0x4E8
#define MC_STREAMID_OVERRIDE_CFG_SCEDMAW	0x4F0
#define MC_STREAMID_OVERRIDE_CFG_APEDMAR	0x4F8
#define MC_STREAMID_OVERRIDE_CFG_APEDMAW	0x500
#define MC_STREAMID_OVERRIDE_CFG_NVDISPLAYR1	0x508
#define MC_STREAMID_OVERRIDE_CFG_VICSRD1	0x510
#define MC_STREAMID_OVERRIDE_CFG_NVDECSRD1	0x518

/*******************************************************************************
 * Macro to calculate Security cfg register addr from StreamID Override register
 ******************************************************************************/
#define MC_STREAMID_OVERRIDE_TO_SECURITY_CFG(addr) (addr + sizeof(uint32_t))

/*******************************************************************************
 * Memory Controller transaction override config registers
 ******************************************************************************/
#define MC_TXN_OVERRIDE_CONFIG_HDAR		0x10a8
#define MC_TXN_OVERRIDE_CONFIG_BPMPW		0x14a0
#define MC_TXN_OVERRIDE_CONFIG_PTCR		0x1000
#define MC_TXN_OVERRIDE_CONFIG_NVDISPLAYR	0x1490
#define MC_TXN_OVERRIDE_CONFIG_EQOSW		0x1478
#define MC_TXN_OVERRIDE_CONFIG_NVJPGSWR		0x13f8
#define MC_TXN_OVERRIDE_CONFIG_ISPRA		0x1220
#define MC_TXN_OVERRIDE_CONFIG_SDMMCWAA		0x1328
#define MC_TXN_OVERRIDE_CONFIG_VICSRD		0x1360
#define MC_TXN_OVERRIDE_CONFIG_MPCOREW		0x11c8
#define MC_TXN_OVERRIDE_CONFIG_GPUSRD		0x12c0
#define MC_TXN_OVERRIDE_CONFIG_AXISR		0x1460
#define MC_TXN_OVERRIDE_CONFIG_SCEDMAW		0x14f0
#define MC_TXN_OVERRIDE_CONFIG_SDMMCW		0x1330
#define MC_TXN_OVERRIDE_CONFIG_EQOSR		0x1470
#define MC_TXN_OVERRIDE_CONFIG_APEDMAR		0x14f8
#define MC_TXN_OVERRIDE_CONFIG_NVENCSRD		0x10e0
#define MC_TXN_OVERRIDE_CONFIG_SDMMCRAB		0x1318
#define MC_TXN_OVERRIDE_CONFIG_VICSRD1		0x1510
#define MC_TXN_OVERRIDE_CONFIG_BPMPDMAR		0x14a8
#define MC_TXN_OVERRIDE_CONFIG_VIW		0x1390
#define MC_TXN_OVERRIDE_CONFIG_SDMMCRAA		0x1308
#define MC_TXN_OVERRIDE_CONFIG_AXISW		0x1468
#define MC_TXN_OVERRIDE_CONFIG_XUSB_DEVR	0x1260
#define MC_TXN_OVERRIDE_CONFIG_UFSHCR		0x1480
#define MC_TXN_OVERRIDE_CONFIG_TSECSWR		0x12a8
#define MC_TXN_OVERRIDE_CONFIG_GPUSWR		0x12c8
#define MC_TXN_OVERRIDE_CONFIG_SATAR		0x10f8
#define MC_TXN_OVERRIDE_CONFIG_XUSB_HOSTW	0x1258
#define MC_TXN_OVERRIDE_CONFIG_TSECSWRB		0x1438
#define MC_TXN_OVERRIDE_CONFIG_GPUSRD2		0x1440
#define MC_TXN_OVERRIDE_CONFIG_SCEDMAR		0x14e8
#define MC_TXN_OVERRIDE_CONFIG_GPUSWR2		0x1448
#define MC_TXN_OVERRIDE_CONFIG_AONDMAW		0x14d0
#define MC_TXN_OVERRIDE_CONFIG_APEDMAW		0x1500
#define MC_TXN_OVERRIDE_CONFIG_AONW		0x14c0
#define MC_TXN_OVERRIDE_CONFIG_HOST1XDMAR	0x10b0
#define MC_TXN_OVERRIDE_CONFIG_ETRR		0x1420
#define MC_TXN_OVERRIDE_CONFIG_SESWR		0x1408
#define MC_TXN_OVERRIDE_CONFIG_NVJPGSRD		0x13f0
#define MC_TXN_OVERRIDE_CONFIG_NVDECSRD		0x13c0
#define MC_TXN_OVERRIDE_CONFIG_TSECSRDB		0x1430
#define MC_TXN_OVERRIDE_CONFIG_BPMPDMAW		0x14b0
#define MC_TXN_OVERRIDE_CONFIG_APER		0x13d0
#define MC_TXN_OVERRIDE_CONFIG_NVDECSRD1	0x1518
#define MC_TXN_OVERRIDE_CONFIG_XUSB_HOSTR	0x1250
#define MC_TXN_OVERRIDE_CONFIG_ISPWA		0x1230
#define MC_TXN_OVERRIDE_CONFIG_SESRD		0x1400
#define MC_TXN_OVERRIDE_CONFIG_SCER		0x14d8
#define MC_TXN_OVERRIDE_CONFIG_AONR		0x14b8
#define MC_TXN_OVERRIDE_CONFIG_MPCORER		0x1138
#define MC_TXN_OVERRIDE_CONFIG_SDMMCWA		0x1320
#define MC_TXN_OVERRIDE_CONFIG_HDAW		0x11a8
#define MC_TXN_OVERRIDE_CONFIG_NVDECSWR		0x13c8
#define MC_TXN_OVERRIDE_CONFIG_UFSHCW		0x1488
#define MC_TXN_OVERRIDE_CONFIG_AONDMAR		0x14c8
#define MC_TXN_OVERRIDE_CONFIG_SATAW		0x11e8
#define MC_TXN_OVERRIDE_CONFIG_ETRW		0x1428
#define MC_TXN_OVERRIDE_CONFIG_VICSWR		0x1368
#define MC_TXN_OVERRIDE_CONFIG_NVENCSWR		0x1158
#define MC_TXN_OVERRIDE_CONFIG_AFIR		0x1070
#define MC_TXN_OVERRIDE_CONFIG_SDMMCWAB		0x1338
#define MC_TXN_OVERRIDE_CONFIG_SDMMCRA		0x1300
#define MC_TXN_OVERRIDE_CONFIG_NVDISPLAYR1	0x1508
#define MC_TXN_OVERRIDE_CONFIG_ISPWB		0x1238
#define MC_TXN_OVERRIDE_CONFIG_BPMPR		0x1498
#define MC_TXN_OVERRIDE_CONFIG_APEW		0x13d8
#define MC_TXN_OVERRIDE_CONFIG_SDMMCR		0x1310
#define MC_TXN_OVERRIDE_CONFIG_XUSB_DEVW	0x1268
#define MC_TXN_OVERRIDE_CONFIG_TSECSRD		0x12a0
#define MC_TXN_OVERRIDE_CONFIG_AFIW		0x1188
#define MC_TXN_OVERRIDE_CONFIG_SCEW		0x14e0

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

#define OVERRIDE_DISABLE				1
#define OVERRIDE_ENABLE					0
#define CLIENT_FLAG_SECURE				0
#define CLIENT_FLAG_NON_SECURE				1
#define CLIENT_INPUTS_OVERRIDE				1
#define CLIENT_INPUTS_NO_OVERRIDE			0

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
#define MC_SMMU_BYPASS_CONFIG			0x1820
#define MC_SMMU_BYPASS_CTRL_MASK		0x3
#define MC_SMMU_BYPASS_CTRL_SHIFT		0
#define MC_SMMU_CTRL_TBU_BYPASS_ALL		(0 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_RSVD			(1 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID	(2 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_NONE		(3 << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT	(1 << 31)
#define MC_SMMU_BYPASS_CONFIG_SETTINGS		(MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT | \
						 MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID)

#define MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_CGID	(1 << 0)
#define MC_TXN_OVERRIDE_CONFIG_COH_PATH_OVERRIDE_SO_DEV	(2 << 4)
#define MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_SO_DEV_CGID_SO_DEV_CLIENT	(1 << 12)

/*******************************************************************************
 * Non-SO_DEV transactions override values for CGID_TAG bitfield for the
 * MC_TXN_OVERRIDE_CONFIG_{module} registers
 ******************************************************************************/
#define MC_TXN_OVERRIDE_CGID_TAG_DEFAULT	0
#define MC_TXN_OVERRIDE_CGID_TAG_CLIENT_AXI_ID	1
#define MC_TXN_OVERRIDE_CGID_TAG_ZERO		2
#define MC_TXN_OVERRIDE_CGID_TAG_ADR		3
#define MC_TXN_OVERRIDE_CGID_TAG_MASK		3

/*******************************************************************************
 * Memory Controller Reset Control registers
 ******************************************************************************/
#define MC_CLIENT_HOTRESET_CTRL0			0x200
#define  MC_CLIENT_HOTRESET_CTRL0_RESET_VAL		0
#define  MC_CLIENT_HOTRESET_CTRL0_AFI_FLUSH_ENB		(1 << 0)
#define  MC_CLIENT_HOTRESET_CTRL0_HC_FLUSH_ENB		(1 << 6)
#define  MC_CLIENT_HOTRESET_CTRL0_HDA_FLUSH_ENB		(1 << 7)
#define  MC_CLIENT_HOTRESET_CTRL0_ISP2_FLUSH_ENB	(1 << 8)
#define  MC_CLIENT_HOTRESET_CTRL0_MPCORE_FLUSH_ENB	(1 << 9)
#define  MC_CLIENT_HOTRESET_CTRL0_NVENC_FLUSH_ENB	(1 << 11)
#define  MC_CLIENT_HOTRESET_CTRL0_SATA_FLUSH_ENB	(1 << 15)
#define  MC_CLIENT_HOTRESET_CTRL0_VI_FLUSH_ENB		(1 << 17)
#define  MC_CLIENT_HOTRESET_CTRL0_VIC_FLUSH_ENB		(1 << 18)
#define  MC_CLIENT_HOTRESET_CTRL0_XUSB_HOST_FLUSH_ENB	(1 << 19)
#define  MC_CLIENT_HOTRESET_CTRL0_XUSB_DEV_FLUSH_ENB	(1 << 20)
#define  MC_CLIENT_HOTRESET_CTRL0_TSEC_FLUSH_ENB	(1 << 22)
#define  MC_CLIENT_HOTRESET_CTRL0_SDMMC1A_FLUSH_ENB	(1 << 29)
#define  MC_CLIENT_HOTRESET_CTRL0_SDMMC2A_FLUSH_ENB	(1 << 30)
#define  MC_CLIENT_HOTRESET_CTRL0_SDMMC3A_FLUSH_ENB	(1 << 31)
#define MC_CLIENT_HOTRESET_STATUS0			0x204
#define MC_CLIENT_HOTRESET_CTRL1			0x970
#define  MC_CLIENT_HOTRESET_CTRL1_RESET_VAL		0
#define  MC_CLIENT_HOTRESET_CTRL1_SDMMC4A_FLUSH_ENB	(1 << 0)
#define  MC_CLIENT_HOTRESET_CTRL1_GPU_FLUSH_ENB		(1 << 2)
#define  MC_CLIENT_HOTRESET_CTRL1_NVDEC_FLUSH_ENB	(1 << 5)
#define  MC_CLIENT_HOTRESET_CTRL1_APE_FLUSH_ENB		(1 << 6)
#define  MC_CLIENT_HOTRESET_CTRL1_SE_FLUSH_ENB		(1 << 7)
#define  MC_CLIENT_HOTRESET_CTRL1_NVJPG_FLUSH_ENB	(1 << 8)
#define  MC_CLIENT_HOTRESET_CTRL1_ETR_FLUSH_ENB		(1 << 12)
#define  MC_CLIENT_HOTRESET_CTRL1_TSECB_FLUSH_ENB	(1 << 13)
#define  MC_CLIENT_HOTRESET_CTRL1_AXIS_FLUSH_ENB	(1 << 18)
#define  MC_CLIENT_HOTRESET_CTRL1_EQOS_FLUSH_ENB	(1 << 19)
#define  MC_CLIENT_HOTRESET_CTRL1_UFSHC_FLUSH_ENB	(1 << 20)
#define  MC_CLIENT_HOTRESET_CTRL1_NVDISPLAY_FLUSH_ENB	(1 << 21)
#define  MC_CLIENT_HOTRESET_CTRL1_BPMP_FLUSH_ENB	(1 << 22)
#define  MC_CLIENT_HOTRESET_CTRL1_AON_FLUSH_ENB		(1 << 23)
#define  MC_CLIENT_HOTRESET_CTRL1_SCE_FLUSH_ENB		(1 << 24)
#define MC_CLIENT_HOTRESET_STATUS1			0x974

/*******************************************************************************
 * Memory Controller's PCFIFO client configuration registers
 ******************************************************************************/
#define MC_PCFIFO_CLIENT_CONFIG1			0xdd4
#define  MC_PCFIFO_CLIENT_CONFIG1_RESET_VAL		0x20000
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_AFIW_UNORDERED	(0 << 17)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_AFIW_MASK	(1 << 17)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_HDAW_UNORDERED	(0 << 21)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_HDAW_MASK	(1 << 21)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_SATAW_UNORDERED (0 << 29)
#define  MC_PCFIFO_CLIENT_CONFIG1_PCFIFO_SATAW_MASK	(1 << 29)

#define MC_PCFIFO_CLIENT_CONFIG2			0xdd8
#define  MC_PCFIFO_CLIENT_CONFIG2_RESET_VAL		0x20000
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_HOSTW_UNORDERED	(0 << 11)
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_HOSTW_MASK	(1 << 11)
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_DEVW_UNORDERED	(0 << 13)
#define  MC_PCFIFO_CLIENT_CONFIG2_PCFIFO_XUSB_DEVW_MASK	(1 << 13)

#define MC_PCFIFO_CLIENT_CONFIG3			0xddc
#define  MC_PCFIFO_CLIENT_CONFIG3_RESET_VAL		0
#define  MC_PCFIFO_CLIENT_CONFIG3_PCFIFO_SDMMCWAB_UNORDERED	(0 << 7)
#define  MC_PCFIFO_CLIENT_CONFIG3_PCFIFO_SDMMCWAB_MASK	(1 << 7)

#define MC_PCFIFO_CLIENT_CONFIG4		0xde0
#define  MC_PCFIFO_CLIENT_CONFIG4_RESET_VAL	0
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SESWR_UNORDERED (0 << 1)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SESWR_MASK	(1 << 1)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_ETRW_UNORDERED	(0 << 5)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_ETRW_MASK	(1 << 5)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AXISW_UNORDERED (0 << 13)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AXISW_MASK	(1 << 13)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_EQOSW_UNORDERED (0 << 15)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_EQOSW_MASK	(1 << 15)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_UFSHCW_UNORDERED	(0 << 17)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_UFSHCW_MASK	(1 << 17)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_BPMPDMAW_UNORDERED	(0 << 22)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_BPMPDMAW_MASK	(1 << 22)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AONDMAW_UNORDERED	(0 << 26)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_AONDMAW_MASK	(1 << 26)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SCEDMAW_UNORDERED	(0 << 30)
#define  MC_PCFIFO_CLIENT_CONFIG4_PCFIFO_SCEDMAW_MASK	(1 << 30)

#define MC_PCFIFO_CLIENT_CONFIG5		0xbf4
#define  MC_PCFIFO_CLIENT_CONFIG5_RESET_VAL	0
#define  MC_PCFIFO_CLIENT_CONFIG5_PCFIFO_APEDMAW_UNORDERED	(0 << 0)
#define  MC_PCFIFO_CLIENT_CONFIG5_PCFIFO_APEDMAW_MASK	(1 << 0)

/*******************************************************************************
 * Memory Controller's SMMU client configuration registers
 ******************************************************************************/
#define MC_SMMU_CLIENT_CONFIG1				0x44
#define  MC_SMMU_CLIENT_CONFIG1_RESET_VAL		0x20000
#define  MC_SMMU_CLIENT_CONFIG1_AFIW_UNORDERED		(0 << 17)
#define  MC_SMMU_CLIENT_CONFIG1_AFIW_MASK		(1 << 17)
#define  MC_SMMU_CLIENT_CONFIG1_HDAW_UNORDERED		(0 << 21)
#define  MC_SMMU_CLIENT_CONFIG1_HDAW_MASK		(1 << 21)
#define  MC_SMMU_CLIENT_CONFIG1_SATAW_UNORDERED		(0 << 29)
#define  MC_SMMU_CLIENT_CONFIG1_SATAW_MASK		(1 << 29)

#define MC_SMMU_CLIENT_CONFIG2				0x48
#define  MC_SMMU_CLIENT_CONFIG2_RESET_VAL		0x20000
#define  MC_SMMU_CLIENT_CONFIG2_XUSB_HOSTW_UNORDERED	(0 << 11)
#define  MC_SMMU_CLIENT_CONFIG2_XUSB_HOSTW_MASK		(1 << 11)
#define  MC_SMMU_CLIENT_CONFIG2_XUSB_DEVW_UNORDERED	(0 << 13)
#define  MC_SMMU_CLIENT_CONFIG2_XUSB_DEVW_MASK		(1 << 13)

#define MC_SMMU_CLIENT_CONFIG3				0x4c
#define  MC_SMMU_CLIENT_CONFIG3_RESET_VAL		0
#define  MC_SMMU_CLIENT_CONFIG3_SDMMCWAB_UNORDERED	(0 << 7)
#define  MC_SMMU_CLIENT_CONFIG3_SDMMCWAB_MASK		(1 << 7)

#define MC_SMMU_CLIENT_CONFIG4				0xb9c
#define  MC_SMMU_CLIENT_CONFIG4_RESET_VAL		0
#define  MC_SMMU_CLIENT_CONFIG4_SESWR_UNORDERED		(0 << 1)
#define  MC_SMMU_CLIENT_CONFIG4_SESWR_MASK		(1 << 1)
#define  MC_SMMU_CLIENT_CONFIG4_ETRW_UNORDERED		(0 << 5)
#define  MC_SMMU_CLIENT_CONFIG4_ETRW_MASK		(1 << 5)
#define  MC_SMMU_CLIENT_CONFIG4_AXISW_UNORDERED		(0 << 13)
#define  MC_SMMU_CLIENT_CONFIG4_AXISW_MASK		(1 << 13)
#define  MC_SMMU_CLIENT_CONFIG4_EQOSW_UNORDERED		(0 << 15)
#define  MC_SMMU_CLIENT_CONFIG4_EQOSW_MASK		(1 << 15)
#define  MC_SMMU_CLIENT_CONFIG4_UFSHCW_UNORDERED	(0 << 17)
#define  MC_SMMU_CLIENT_CONFIG4_UFSHCW_MASK		(1 << 17)
#define  MC_SMMU_CLIENT_CONFIG4_BPMPDMAW_UNORDERED	(0 << 22)
#define  MC_SMMU_CLIENT_CONFIG4_BPMPDMAW_MASK		(1 << 22)
#define  MC_SMMU_CLIENT_CONFIG4_AONDMAW_UNORDERED	(0 << 26)
#define  MC_SMMU_CLIENT_CONFIG4_AONDMAW_MASK		(1 << 26)
#define  MC_SMMU_CLIENT_CONFIG4_SCEDMAW_UNORDERED	(0 << 30)
#define  MC_SMMU_CLIENT_CONFIG4_SCEDMAW_MASK		(1 << 30)

#define MC_SMMU_CLIENT_CONFIG5				0xbac
#define  MC_SMMU_CLIENT_CONFIG5_RESET_VAL		0
#define  MC_SMMU_CLIENT_CONFIG5_APEDMAW_UNORDERED	(0 << 0)
#define  MC_SMMU_CLIENT_CONFIG5_APEDMAW_MASK	(1 << 0)

#ifndef __ASSEMBLY__

#include <mmio.h>

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
	(~MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_MASK | \
	 MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_UNORDERED)

#define mc_set_smmu_unordered_boot_so_mss(id, client) \
	(~MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_MASK | \
	 MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_UNORDERED)

#define mc_set_tsa_passthrough(client) \
	{ \
		mmio_write_32(TEGRA_TSA_BASE + TSA_CONFIG_STATIC0_CSW_##client, \
			(TSA_CONFIG_STATIC0_CSW_##client##_RESET & \
			 ~TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK) | \
			TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU); \
	}

#define mc_set_forced_coherent_cfg(client) \
	{ \
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_##client, \
			MC_TXN_OVERRIDE_CONFIG_COH_PATH_OVERRIDE_SO_DEV); \
	}

#define mc_set_forced_coherent_so_dev_cfg(client) \
	{ \
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_##client, \
			MC_TXN_OVERRIDE_CONFIG_COH_PATH_OVERRIDE_SO_DEV | \
			MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_SO_DEV_CGID_SO_DEV_CLIENT); \
	}

#define mc_set_forced_coherent_axid_so_dev_cfg(client) \
	{ \
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_##client, \
			MC_TXN_OVERRIDE_CONFIG_COH_PATH_OVERRIDE_SO_DEV | \
			MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_CGID | \
			MC_TXN_OVERRIDE_CONFIG_AXID_OVERRIDE_SO_DEV_CGID_SO_DEV_CLIENT); \
	}

/*******************************************************************************
 * Handler to read memory configuration settings
 *
 * Implemented by SoCs under tegra/soc/txxx
 ******************************************************************************/
tegra_mc_settings_t *tegra_get_mc_settings(void);

#endif /* __ASSMEBLY__ */

#endif /* __MEMCTRLV2_H__ */
