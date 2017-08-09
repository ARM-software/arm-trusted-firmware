/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_DEF_H__
#define __TEGRA_DEF_H__

#include <lib/utils_def.h>

/*******************************************************************************
 * These values are used by the PSCI implementation during the `CPU_SUSPEND`
 * and `SYSTEM_SUSPEND` calls as the `state-id` field in the 'power state'
 * parameter.
 ******************************************************************************/
#define PSTATE_ID_CORE_IDLE		6
#define PSTATE_ID_CORE_POWERDN		7
#define PSTATE_ID_SOC_POWERDN		2

/*******************************************************************************
 * Platform power states (used by PSCI framework)
 *
 * - PLAT_MAX_RET_STATE should be less than lowest PSTATE_ID
 * - PLAT_MAX_OFF_STATE should be greater than the highest PSTATE_ID
 ******************************************************************************/
#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		8

/*******************************************************************************
 * Secure IRQ definitions
 ******************************************************************************/
#define TEGRA186_MAX_SEC_IRQS		5
#define TEGRA186_BPMP_WDT_IRQ		46
#define TEGRA186_SPE_WDT_IRQ		47
#define TEGRA186_SCE_WDT_IRQ		48
#define TEGRA186_TOP_WDT_IRQ		49
#define TEGRA186_AON_WDT_IRQ		50

#define TEGRA186_SEC_IRQ_TARGET_MASK	0xFF /* 8 Carmel */

/*******************************************************************************
 * Tegra Miscellanous register constants
 ******************************************************************************/
#define TEGRA_MISC_BASE				0x00100000U

#define HARDWARE_REVISION_OFFSET	0x4U
#define MISCREG_EMU_REVID			0x3160U
#define  BOARD_MASK_BITS			0xFFU
#define  BOARD_SHIFT_BITS			24U
#define MISCREG_PFCFG				0x200CU

/*******************************************************************************
 * Tegra TSA Controller constants
 ******************************************************************************/
#define TEGRA_TSA_BASE			0x02000000

#define TSA_CONFIG_STATIC0_CSW_SESWR		0x1010
#define  TSA_CONFIG_STATIC0_CSW_SESWR_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_ETRW		0xD034
#define  TSA_CONFIG_STATIC0_CSW_ETRW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_SDMMCWAB		0x3020
#define  TSA_CONFIG_STATIC0_CSW_SDMMCWAB_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_AXISW		0x8008
#define  TSA_CONFIG_STATIC0_CSW_AXISW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_HDAW		0xD008
#define  TSA_CONFIG_STATIC0_CSW_HDAW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_AONDMAW		0xE018
#define  TSA_CONFIG_STATIC0_CSW_AONDMAW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_SCEDMAW		0x9008
#define  TSA_CONFIG_STATIC0_CSW_SCEDMAW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_BPMPDMAW		0x9028
#define  TSA_CONFIG_STATIC0_CSW_BPMPDMAW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_APEDMAW		0xB008
#define  TSA_CONFIG_STATIC0_CSW_APEDMAW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_UFSHCW		0x6008
#define  TSA_CONFIG_STATIC0_CSW_UFSHCW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_AFIW		0xF008
#define  TSA_CONFIG_STATIC0_CSW_AFIW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_SATAW		0x4008
#define  TSA_CONFIG_STATIC0_CSW_SATAW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_EQOSW		0x3038
#define  TSA_CONFIG_STATIC0_CSW_EQOSW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_XUSB_DEVW	0x6018
#define  TSA_CONFIG_STATIC0_CSW_XUSB_DEVW_RESET	0x1100
#define TSA_CONFIG_STATIC0_CSW_XUSB_HOSTW	0x6028
#define  TSA_CONFIG_STATIC0_CSW_XUSB_HOSTW_RESET 0x1100

#define TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK	(0x3 << 11)
#define TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU	(0 << 11)

/*******************************************************************************
 * Tegra Memory Controller constants
 ******************************************************************************/
#define TEGRA_MC_STREAMID_BASE		0x02C00000
#define TEGRA_MC_BASE			0x02C10000

/* General Security Carveout register macros */
#define MC_GSC_CONFIG_REGS_SIZE		0x40
#define MC_GSC_LOCK_CFG_SETTINGS_BIT	(1 << 1)
#define MC_GSC_ENABLE_TZ_LOCK_BIT	(1 << 0)
#define MC_GSC_SIZE_RANGE_4KB_SHIFT	27
#define MC_GSC_BASE_LO_SHIFT		12
#define MC_GSC_BASE_LO_MASK		0xFFFFF
#define MC_GSC_BASE_HI_SHIFT		0
#define MC_GSC_BASE_HI_MASK		3

/* TZDRAM carveout configuration registers */
#define MC_SECURITY_CFG0_0		0x70
#define MC_SECURITY_CFG1_0		0x74
#define MC_SECURITY_CFG3_0		0x9BC

#define MC_SECURITY_BOM_MASK		(U(0xFFF) << 20)
#define MC_SECURITY_SIZE_MB_MASK	(U(0x1FFF) << 0)
#define MC_SECURITY_BOM_HI_MASK		(U(0x3) << 0)

/* Video Memory carveout configuration registers */
#define MC_VIDEO_PROTECT_BASE_HI	0x978
#define MC_VIDEO_PROTECT_BASE_LO	0x648
#define MC_VIDEO_PROTECT_SIZE_MB	0x64c

/*
 * Carveout (MC_SECURITY_CARVEOUT24) registers used to clear the
 * non-overlapping Video memory region
 */
#define MC_VIDEO_PROTECT_CLEAR_CFG	0x25A0
#define MC_VIDEO_PROTECT_CLEAR_BASE_LO	0x25A4
#define MC_VIDEO_PROTECT_CLEAR_BASE_HI	0x25A8
#define MC_VIDEO_PROTECT_CLEAR_SIZE	0x25AC
#define MC_VIDEO_PROTECT_CLEAR_ACCESS_CFG0	0x25B0

/* TZRAM carveout (MC_SECURITY_CARVEOUT11) configuration registers */
#define MC_TZRAM_CARVEOUT_CFG		0x2190
#define MC_TZRAM_BASE_LO		0x2194
#define MC_TZRAM_BASE_HI		0x2198
#define MC_TZRAM_SIZE			0x219C
#define MC_TZRAM_CLIENT_ACCESS_CFG0	0x21A0

/* Memory Controller Reset Control registers */
#define  MC_CLIENT_HOTRESET_CTRL1_VIFAL_FLUSH_ENB	(1 << 27)
#define  MC_CLIENT_HOTRESET_CTRL1_DLAA_FLUSH_ENB	(1 << 28)
#define  MC_CLIENT_HOTRESET_CTRL1_DLA1A_FLUSH_ENB	(1 << 29)
#define  MC_CLIENT_HOTRESET_CTRL1_PVA0A_FLUSH_ENB	(1 << 30)
#define  MC_CLIENT_HOTRESET_CTRL1_PVA1A_FLUSH_ENB	(1 << 31)

/*******************************************************************************
 * Tegra UART Controller constants
 ******************************************************************************/
#define TEGRA_UARTA_BASE		0x03100000
#define TEGRA_UARTB_BASE		0x03110000
#define TEGRA_UARTC_BASE		0x0C280000
#define TEGRA_UARTD_BASE		0x03130000
#define TEGRA_UARTE_BASE		0x03140000
#define TEGRA_UARTF_BASE		0x03150000
#define TEGRA_UARTG_BASE		0x0C290000

/*******************************************************************************
 * Tegra Fuse Controller related constants
 ******************************************************************************/
#define TEGRA_FUSE_BASE			0x03820000
#define  OPT_SUBREVISION		0x248
#define  SUBREVISION_MASK		0xF

/*******************************************************************************
 * GICv2 & interrupt handling related constants
 ******************************************************************************/
#define TEGRA_GICD_BASE			0x03881000
#define TEGRA_GICC_BASE			0x03882000

/*******************************************************************************
 * Security Engine related constants
 ******************************************************************************/
#define TEGRA_SE0_BASE			0x03AC0000
#define  SE_MUTEX_WATCHDOG_NS_LIMIT	0x6C
#define TEGRA_PKA1_BASE			0x03AD0000
#define  PKA_MUTEX_WATCHDOG_NS_LIMIT	0x8144
#define TEGRA_RNG1_BASE			0x03AE0000
#define  RNG_MUTEX_WATCHDOG_NS_LIMIT	0xFE0

/*******************************************************************************
 * Tegra micro-seconds timer constants
 ******************************************************************************/
#define TEGRA_TMRUS_BASE		0x0C2E0000
#define TEGRA_TMRUS_SIZE		0x10000

/*******************************************************************************
 * Tegra Power Mgmt Controller constants
 ******************************************************************************/
#define TEGRA_PMC_BASE			0x0C360000

/*******************************************************************************
 * Tegra scratch registers constants
 ******************************************************************************/
#define TEGRA_SCRATCH_BASE		0x0C390000
#define  SECURE_SCRATCH_RSV1_LO		0x06C
#define  SECURE_SCRATCH_RSV1_HI		0x070
#define  SECURE_SCRATCH_RSV6		0x094
#define  SECURE_SCRATCH_RSV11_LO	0x0BC
#define  SECURE_SCRATCH_RSV11_HI	0x0C0
#define  SECURE_SCRATCH_RSV53_LO	0x20C
#define  SECURE_SCRATCH_RSV53_HI	0x210
#define  SECURE_SCRATCH_RSV54_HI	0x218
#define  SECURE_SCRATCH_RSV55_LO	0x21C
#define  SECURE_SCRATCH_RSV55_HI	0x220

/*******************************************************************************
 * Tegra Memory Mapped Control Register Access Bus constants
 ******************************************************************************/
#define TEGRA_MMCRAB_BASE		0x0E000000

/*******************************************************************************
 * Tegra SMMU Controller constants
 ******************************************************************************/
#define TEGRA_SMMU0_BASE		0x12000000
#define TEGRA_SMMU1_BASE		0x11000000
#define TEGRA_SMMU2_BASE		0x10000000

/*******************************************************************************
 * Tegra TZRAM constants
 ******************************************************************************/
#define TEGRA_TZRAM_BASE		0x40000000
#define TEGRA_TZRAM_SIZE		0x40000

/*******************************************************************************
 * Tegra Clock and Reset Controller constants
 ******************************************************************************/
#define TEGRA_CAR_RESET_BASE		0x200000000
#define TEGRA_GPU_RESET_REG_OFFSET	0x18UL
#define  GPU_RESET_BIT			(1UL << 0)

/*******************************************************************************
 * Stream ID Override Config registers
 ******************************************************************************/
#define MC_STREAMID_OVERRIDE_CFG_ISPFALR	0x228U
#define MC_STREAMID_OVERRIDE_CFG_AXIAPR		0x410U
#define MC_STREAMID_OVERRIDE_CFG_AXIAPW		0x418U
#define MC_STREAMID_OVERRIDE_CFG_MIU0R		0x530U
#define MC_STREAMID_OVERRIDE_CFG_MIU0W		0x538U
#define MC_STREAMID_OVERRIDE_CFG_MIU1R		0x540U
#define MC_STREAMID_OVERRIDE_CFG_MIU1W		0x548U
#define MC_STREAMID_OVERRIDE_CFG_MIU2R		0x570U
#define MC_STREAMID_OVERRIDE_CFG_MIU2W		0x578U
#define MC_STREAMID_OVERRIDE_CFG_MIU3R		0x580U
#define MC_STREAMID_OVERRIDE_CFG_MIU3W		0x588U
#define MC_STREAMID_OVERRIDE_CFG_VIFALR		0x5E0U
#define MC_STREAMID_OVERRIDE_CFG_VIFALW		0x5E8U
#define MC_STREAMID_OVERRIDE_CFG_DLA0RDA	0x5F0U
#define MC_STREAMID_OVERRIDE_CFG_DLA0FALRDB	0x5F8U
#define MC_STREAMID_OVERRIDE_CFG_DLA0WRA	0x600U
#define MC_STREAMID_OVERRIDE_CFG_DLA0FALWRB	0x608U
#define MC_STREAMID_OVERRIDE_CFG_DLA1RDA	0x610U
#define MC_STREAMID_OVERRIDE_CFG_DLA1FALRDB	0x618U
#define MC_STREAMID_OVERRIDE_CFG_DLA1WRA	0x620U
#define MC_STREAMID_OVERRIDE_CFG_DLA1FALWRB	0x628U
#define MC_STREAMID_OVERRIDE_CFG_PVA0RDA	0x630U
#define MC_STREAMID_OVERRIDE_CFG_PVA0RDB	0x638U
#define MC_STREAMID_OVERRIDE_CFG_PVA0RDC	0x640U
#define MC_STREAMID_OVERRIDE_CFG_PVA0WRA	0x648U
#define MC_STREAMID_OVERRIDE_CFG_PVA0WRB	0x650U
#define MC_STREAMID_OVERRIDE_CFG_PVA0WRC	0x658U
#define MC_STREAMID_OVERRIDE_CFG_PVA1RDA	0x660U
#define MC_STREAMID_OVERRIDE_CFG_PVA1RDB	0x668U
#define MC_STREAMID_OVERRIDE_CFG_PVA1RDC	0x670U
#define MC_STREAMID_OVERRIDE_CFG_PVA1WRA	0x678U
#define MC_STREAMID_OVERRIDE_CFG_PVA1WRB	0x680U
#define MC_STREAMID_OVERRIDE_CFG_PVA1WRC	0x688U
#define MC_STREAMID_OVERRIDE_CFG_RCER		0x690U
#define MC_STREAMID_OVERRIDE_CFG_RCEW		0x698U
#define MC_STREAMID_OVERRIDE_CFG_RCEDMAR	0x6A0U
#define MC_STREAMID_OVERRIDE_CFG_RCEDMAW	0x6A8U
#define MC_STREAMID_OVERRIDE_CFG_NVENC1SRD	0x6B0U
#define MC_STREAMID_OVERRIDE_CFG_NVENC1SWR	0x6B8U
#define MC_STREAMID_OVERRIDE_CFG_PCIE0R		0x6C0U
#define MC_STREAMID_OVERRIDE_CFG_PCIE0W		0x6C8U
#define MC_STREAMID_OVERRIDE_CFG_PCIE1R		0x6D0U
#define MC_STREAMID_OVERRIDE_CFG_PCIE1W		0x6D8U
#define MC_STREAMID_OVERRIDE_CFG_PCIE2AR	0x6E0U
#define MC_STREAMID_OVERRIDE_CFG_PCIE2AW	0x6E8U
#define MC_STREAMID_OVERRIDE_CFG_PCIE3R		0x6F0U
#define MC_STREAMID_OVERRIDE_CFG_PCIE3W		0x6F8U
#define MC_STREAMID_OVERRIDE_CFG_PCIE4R		0x700U
#define MC_STREAMID_OVERRIDE_CFG_PCIE4W		0x708U
#define MC_STREAMID_OVERRIDE_CFG_PCIE5R		0x710U
#define MC_STREAMID_OVERRIDE_CFG_PCIE5W		0x718U
#define MC_STREAMID_OVERRIDE_CFG_ISPFALW	0x720U
#define MC_STREAMID_OVERRIDE_CFG_DLA0RDA1	0x748U
#define MC_STREAMID_OVERRIDE_CFG_DLA1RDA1	0x750U
#define MC_STREAMID_OVERRIDE_CFG_PVA0RDA1	0x758U
#define MC_STREAMID_OVERRIDE_CFG_PVA0RDB1	0x760U
#define MC_STREAMID_OVERRIDE_CFG_PVA1RDA1	0x768U
#define MC_STREAMID_OVERRIDE_CFG_PVA1RDB1	0x770U
#define MC_STREAMID_OVERRIDE_CFG_PCIE5R1	0x778U
#define MC_STREAMID_OVERRIDE_CFG_NVENCSRD1	0x780U
#define MC_STREAMID_OVERRIDE_CFG_NVENC1SRD1	0x788U
#define MC_STREAMID_OVERRIDE_CFG_ISPRA1		0x790U
#define MC_STREAMID_OVERRIDE_CFG_PCIE0R1	0x798U
#define MC_STREAMID_OVERRIDE_CFG_NVDEC1SRD	0x7C8U
#define MC_STREAMID_OVERRIDE_CFG_NVDEC1SRD1	0x7D0U
#define MC_STREAMID_OVERRIDE_CFG_NVDEC1SWR	0x7D8U

/*******************************************************************************
 * Memory Controller transaction override config registers
 ******************************************************************************/
#define MC_TXN_OVERRIDE_CONFIG_MIU0R		0x1530
#define MC_TXN_OVERRIDE_CONFIG_MIU0W		0x1538
#define MC_TXN_OVERRIDE_CONFIG_MIU1R		0x1540
#define MC_TXN_OVERRIDE_CONFIG_MIU1W		0x1548
#define MC_TXN_OVERRIDE_CONFIG_MIU2R		0x1570
#define MC_TXN_OVERRIDE_CONFIG_MIU2W		0x1578
#define MC_TXN_OVERRIDE_CONFIG_MIU3R		0x1580
#define MC_TXN_OVERRIDE_CONFIG_MIU3W		0x158C
#define MC_TXN_OVERRIDE_CONFIG_VIFALR		0x15E4
#define MC_TXN_OVERRIDE_CONFIG_VIFALW		0x15EC
#define MC_TXN_OVERRIDE_CONFIG_DLA0RDA		0x15F4
#define MC_TXN_OVERRIDE_CONFIG_DLA0FALRDB	0x15FC
#define MC_TXN_OVERRIDE_CONFIG_DLA0WRA		0x1604
#define MC_TXN_OVERRIDE_CONFIG_DLA0FALWRB	0x160C
#define MC_TXN_OVERRIDE_CONFIG_DLA1RDA		0x1614
#define MC_TXN_OVERRIDE_CONFIG_DLA1FALRDB	0x161C
#define MC_TXN_OVERRIDE_CONFIG_DLA1WRA		0x1624
#define MC_TXN_OVERRIDE_CONFIG_DLA1FALWRB	0x162C
#define MC_TXN_OVERRIDE_CONFIG_PVA0RDA		0x1634
#define MC_TXN_OVERRIDE_CONFIG_PVA0RDB		0x163C
#define MC_TXN_OVERRIDE_CONFIG_PVA0RDC		0x1644
#define MC_TXN_OVERRIDE_CONFIG_PVA0WRA		0x164C
#define MC_TXN_OVERRIDE_CONFIG_PVA0WRB		0x1654
#define MC_TXN_OVERRIDE_CONFIG_PVA0WRC		0x165C
#define MC_TXN_OVERRIDE_CONFIG_PVA1RDA		0x1664
#define MC_TXN_OVERRIDE_CONFIG_PVA1RDB		0x166C
#define MC_TXN_OVERRIDE_CONFIG_PVA1RDC		0x1674
#define MC_TXN_OVERRIDE_CONFIG_PVA1WRA		0x167C
#define MC_TXN_OVERRIDE_CONFIG_PVA1WRB		0x1684
#define MC_TXN_OVERRIDE_CONFIG_PVA1WRC		0x168C
#define MC_TXN_OVERRIDE_CONFIG_RCER		0x1694
#define MC_TXN_OVERRIDE_CONFIG_RCEW		0x169C
#define MC_TXN_OVERRIDE_CONFIG_RCEDMAR		0x16A4
#define MC_TXN_OVERRIDE_CONFIG_RCEDMAW		0x16AC
#define MC_TXN_OVERRIDE_CONFIG_NVENC1SRD	0x16B4
#define MC_TXN_OVERRIDE_CONFIG_NVENC1SWR	0x16BC
#define MC_TXN_OVERRIDE_CONFIG_PCIE0R		0x16C4
#define MC_TXN_OVERRIDE_CONFIG_PCIE0W		0x16CC
#define MC_TXN_OVERRIDE_CONFIG_PCIE1R		0x16D4
#define MC_TXN_OVERRIDE_CONFIG_PCIE1W		0x16DC
#define MC_TXN_OVERRIDE_CONFIG_PCIE2AR		0x16E4
#define MC_TXN_OVERRIDE_CONFIG_PCIE2AW		0x16EC
#define MC_TXN_OVERRIDE_CONFIG_PCIE3R		0x16F4
#define MC_TXN_OVERRIDE_CONFIG_PCIE3W		0x16FC
#define MC_TXN_OVERRIDE_CONFIG_PCIE4R		0x1704
#define MC_TXN_OVERRIDE_CONFIG_PCIE4W		0x170C
#define MC_TXN_OVERRIDE_CONFIG_PCIE5R		0x1714
#define MC_TXN_OVERRIDE_CONFIG_PCIE5W		0x171C
#define MC_TXN_OVERRIDE_CONFIG_ISPFALW		0x1724
#define MC_TXN_OVERRIDE_CONFIG_DLA0RDA1		0x174C
#define MC_TXN_OVERRIDE_CONFIG_DLA1RDA1		0x1754
#define MC_TXN_OVERRIDE_CONFIG_PVA0RDA1		0x175C
#define MC_TXN_OVERRIDE_CONFIG_PVA0RDB1		0x1764
#define MC_TXN_OVERRIDE_CONFIG_PVA1RDA1		0x176C
#define MC_TXN_OVERRIDE_CONFIG_PVA1RDB1		0x1774
#define MC_TXN_OVERRIDE_CONFIG_PCIE5R1		0x177C
#define MC_TXN_OVERRIDE_CONFIG_NVENCSRD1	0x1784
#define MC_TXN_OVERRIDE_CONFIG_NVENC1SRD1	0x178C
#define MC_TXN_OVERRIDE_CONFIG_ISPRA1		0x1794
#define MC_TXN_OVERRIDE_CONFIG_PCIE0R1		0x179C
#define MC_TXN_OVERRIDE_CONFIG_NVDEC1SRD	0x17CC
#define MC_TXN_OVERRIDE_CONFIG_NVDEC1SRD1	0x17D4
#define MC_TXN_OVERRIDE_CONFIG_NVDEC1SWR	0x17DC

/*******************************************************************************
 * XUSB PADCTL
 ******************************************************************************/
#define TEGRA_XUSB_PADCTL_BASE			(0x3520000U)
#define TEGRA_XUSB_PADCTL_SIZE			(0x10000U)
#define XUSB_PADCTL_HOST_AXI_STREAMID_PF_0	(0x136cU)
#define XUSB_PADCTL_HOST_AXI_STREAMID_VF_0	(0x1370U)
#define XUSB_PADCTL_HOST_AXI_STREAMID_VF_1	(0x1374U)
#define XUSB_PADCTL_HOST_AXI_STREAMID_VF_2	(0x1378U)
#define XUSB_PADCTL_HOST_AXI_STREAMID_VF_3	(0x137cU)
#define XUSB_PADCTL_DEV_AXI_STREAMID_PF_0	(0x139cU)

/*******************************************************************************
 * XUSB STREAMIDs
 ******************************************************************************/
#define TEGRA_SID_XUSB_HOST			(0x1bU)
#define TEGRA_SID_XUSB_DEV			(0x1cU)
#define TEGRA_SID_XUSB_VF0			(0x5dU)
#define TEGRA_SID_XUSB_VF1			(0x5eU)
#define TEGRA_SID_XUSB_VF2			(0x5fU)
#define TEGRA_SID_XUSB_VF3			(0x60U)

#endif /* __TEGRA_DEF_H__ */
