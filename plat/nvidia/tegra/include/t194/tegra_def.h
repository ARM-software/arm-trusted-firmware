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
#define MC_GSC_ENABLE_CPU_SECURE_BIT    (U(1) << 31)

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
#define MC_TZRAM_CLIENT_ACCESS0_CFG0	U(0x21A0)
#define MC_TZRAM_CLIENT_ACCESS1_CFG0	U(0x21A4)
#define  TZRAM_ALLOW_MPCORER		(U(1) << 7)
#define  TZRAM_ALLOW_MPCOREW		(U(1) << 25)

/* Memory Controller Reset Control registers */
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
#define TEGRA_CAR_RESET_BASE		0x20000000

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
