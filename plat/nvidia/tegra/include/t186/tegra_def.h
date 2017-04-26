/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_DEF_H__
#define __TEGRA_DEF_H__

#include <utils_def.h>

/*******************************************************************************
 * MCE apertures used by the ARI interface
 *
 * Aperture 0 - Cpu0 (ARM Cortex A-57)
 * Aperture 1 - Cpu1 (ARM Cortex A-57)
 * Aperture 2 - Cpu2 (ARM Cortex A-57)
 * Aperture 3 - Cpu3 (ARM Cortex A-57)
 * Aperture 4 - Cpu4 (Denver15)
 * Aperture 5 - Cpu5 (Denver15)
 ******************************************************************************/
#define MCE_ARI_APERTURE_0_OFFSET	U(0x0)
#define MCE_ARI_APERTURE_1_OFFSET	U(0x10000)
#define MCE_ARI_APERTURE_2_OFFSET	U(0x20000)
#define MCE_ARI_APERTURE_3_OFFSET	U(0x30000)
#define MCE_ARI_APERTURE_4_OFFSET	U(0x40000)
#define MCE_ARI_APERTURE_5_OFFSET	U(0x50000)
#define MCE_ARI_APERTURE_OFFSET_MAX	MCE_APERTURE_5_OFFSET

/* number of apertures */
#define MCE_ARI_APERTURES_MAX		U(6)

/* each ARI aperture is 64KB */
#define MCE_ARI_APERTURE_SIZE		U(0x10000)

/*******************************************************************************
 * CPU core id macros for the MCE_ONLINE_CORE ARI
 ******************************************************************************/
#define MCE_CORE_ID_MAX			U(8)
#define MCE_CORE_ID_MASK		U(0x7)

/*******************************************************************************
 * These values are used by the PSCI implementation during the `CPU_SUSPEND`
 * and `SYSTEM_SUSPEND` calls as the `state-id` field in the 'power state'
 * parameter.
 ******************************************************************************/
#define PSTATE_ID_CORE_IDLE		U(6)
#define PSTATE_ID_CORE_POWERDN		U(7)
#define PSTATE_ID_SOC_POWERDN		U(2)

/*******************************************************************************
 * Platform power states (used by PSCI framework)
 *
 * - PLAT_MAX_RET_STATE should be less than lowest PSTATE_ID
 * - PLAT_MAX_OFF_STATE should be greater than the highest PSTATE_ID
 ******************************************************************************/
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(8)

/*******************************************************************************
 * Secure IRQ definitions
 ******************************************************************************/
#define TEGRA186_TOP_WDT_IRQ		U(49)
#define TEGRA186_AON_WDT_IRQ		U(50)

#define TEGRA186_SEC_IRQ_TARGET_MASK	U(0xF3) /* 4 A57 - 2 Denver */

/*******************************************************************************
 * Tegra Miscellanous register constants
 ******************************************************************************/
#define TEGRA_MISC_BASE			U(0x00100000)
#define  HARDWARE_REVISION_OFFSET	U(0x4)

#define  MISCREG_PFCFG			U(0x200C)

/*******************************************************************************
 * Tegra TSA Controller constants
 ******************************************************************************/
#define TEGRA_TSA_BASE			U(0x02400000)

/*******************************************************************************
 * TSA configuration registers
 ******************************************************************************/
#define TSA_CONFIG_STATIC0_CSW_SESWR			U(0x4010)
#define  TSA_CONFIG_STATIC0_CSW_SESWR_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_ETRW			U(0x4038)
#define  TSA_CONFIG_STATIC0_CSW_ETRW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_SDMMCWAB			U(0x5010)
#define  TSA_CONFIG_STATIC0_CSW_SDMMCWAB_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_AXISW			U(0x7008)
#define  TSA_CONFIG_STATIC0_CSW_AXISW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_HDAW			U(0xA008)
#define  TSA_CONFIG_STATIC0_CSW_HDAW_RESET		U(0x100)
#define TSA_CONFIG_STATIC0_CSW_AONDMAW			U(0xB018)
#define  TSA_CONFIG_STATIC0_CSW_AONDMAW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_SCEDMAW			U(0xD018)
#define  TSA_CONFIG_STATIC0_CSW_SCEDMAW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_BPMPDMAW			U(0xD028)
#define  TSA_CONFIG_STATIC0_CSW_BPMPDMAW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_APEDMAW			U(0x12018)
#define  TSA_CONFIG_STATIC0_CSW_APEDMAW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_UFSHCW			U(0x13008)
#define  TSA_CONFIG_STATIC0_CSW_UFSHCW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_AFIW			U(0x13018)
#define  TSA_CONFIG_STATIC0_CSW_AFIW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_SATAW			U(0x13028)
#define  TSA_CONFIG_STATIC0_CSW_SATAW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_EQOSW			U(0x13038)
#define  TSA_CONFIG_STATIC0_CSW_EQOSW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_XUSB_DEVW		U(0x15008)
#define  TSA_CONFIG_STATIC0_CSW_XUSB_DEVW_RESET		U(0x1100)
#define TSA_CONFIG_STATIC0_CSW_XUSB_HOSTW		U(0x15018)
#define  TSA_CONFIG_STATIC0_CSW_XUSB_HOSTW_RESET	U(0x1100)

#define TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK		(U(0x3) << 11)
#define TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU		(U(0) << 11)

/*******************************************************************************
 * Tegra Memory Controller constants
 ******************************************************************************/
#define TEGRA_MC_STREAMID_BASE		U(0x02C00000)
#define TEGRA_MC_BASE			U(0x02C10000)

/* General Security Carveout register macros */
#define MC_GSC_CONFIG_REGS_SIZE		U(0x40)
#define MC_GSC_LOCK_CFG_SETTINGS_BIT	(U(1) << 1)
#define MC_GSC_ENABLE_TZ_LOCK_BIT	(U(1) << 0)
#define MC_GSC_SIZE_RANGE_4KB_SHIFT	U(27)
#define MC_GSC_BASE_LO_SHIFT		U(12)
#define MC_GSC_BASE_LO_MASK		U(0xFFFFF)
#define MC_GSC_BASE_HI_SHIFT		U(0)
#define MC_GSC_BASE_HI_MASK		U(3)

/* TZDRAM carveout configuration registers */
#define MC_SECURITY_CFG0_0		U(0x70)
#define MC_SECURITY_CFG1_0		U(0x74)
#define MC_SECURITY_CFG3_0		U(0x9BC)

/* Video Memory carveout configuration registers */
#define MC_VIDEO_PROTECT_BASE_HI	U(0x978)
#define MC_VIDEO_PROTECT_BASE_LO	U(0x648)
#define MC_VIDEO_PROTECT_SIZE_MB	U(0x64C)

/*
 * Carveout (MC_SECURITY_CARVEOUT24) registers used to clear the
 * non-overlapping Video memory region
 */
#define MC_VIDEO_PROTECT_CLEAR_CFG	U(0x25A0)
#define MC_VIDEO_PROTECT_CLEAR_BASE_LO	U(0x25A4)
#define MC_VIDEO_PROTECT_CLEAR_BASE_HI	U(0x25A8)
#define MC_VIDEO_PROTECT_CLEAR_SIZE	U(0x25AC)
#define MC_VIDEO_PROTECT_CLEAR_ACCESS_CFG0	U(0x25B0)

/* TZRAM carveout (MC_SECURITY_CARVEOUT11) configuration registers */
#define MC_TZRAM_CARVEOUT_CFG		U(0x2190)
#define MC_TZRAM_BASE_LO		U(0x2194)
#define MC_TZRAM_BASE_HI		U(0x2198)
#define MC_TZRAM_SIZE			U(0x219C)
#define MC_TZRAM_CLIENT_ACCESS_CFG0	U(0x21A0)

/*******************************************************************************
 * Tegra UART Controller constants
 ******************************************************************************/
#define TEGRA_UARTA_BASE		U(0x03100000)
#define TEGRA_UARTB_BASE		U(0x03110000)
#define TEGRA_UARTC_BASE		U(0x0C280000)
#define TEGRA_UARTD_BASE		U(0x03130000)
#define TEGRA_UARTE_BASE		U(0x03140000)
#define TEGRA_UARTF_BASE		U(0x03150000)
#define TEGRA_UARTG_BASE		U(0x0C290000)

/*******************************************************************************
 * Tegra Fuse Controller related constants
 ******************************************************************************/
#define TEGRA_FUSE_BASE			U(0x03820000)
#define  OPT_SUBREVISION		U(0x248)
#define  SUBREVISION_MASK		U(0xFF)

/*******************************************************************************
 * GICv2 & interrupt handling related constants
 ******************************************************************************/
#define TEGRA_GICD_BASE			U(0x03881000)
#define TEGRA_GICC_BASE			U(0x03882000)

/*******************************************************************************
 * Security Engine related constants
 ******************************************************************************/
#define TEGRA_SE0_BASE			U(0x03AC0000)
#define  SE_MUTEX_WATCHDOG_NS_LIMIT	U(0x6C)
#define TEGRA_PKA1_BASE			U(0x03AD0000)
#define  PKA_MUTEX_WATCHDOG_NS_LIMIT	U(0x8144)
#define TEGRA_RNG1_BASE			U(0x03AE0000)
#define  RNG_MUTEX_WATCHDOG_NS_LIMIT	U(0xFE0)

/*******************************************************************************
 * Tegra Clock and Reset Controller constants
 ******************************************************************************/
#define TEGRA_CAR_RESET_BASE		U(0x05000000)
#define TEGRA_GPU_RESET_REG_OFFSET	U(0x30)
#define  GPU_RESET_BIT			(U(1) << 0)

/*******************************************************************************
 * Tegra micro-seconds timer constants
 ******************************************************************************/
#define TEGRA_TMRUS_BASE		U(0x0C2E0000)
#define TEGRA_TMRUS_SIZE		U(0x1000)

/*******************************************************************************
 * Tegra Power Mgmt Controller constants
 ******************************************************************************/
#define TEGRA_PMC_BASE			U(0x0C360000)

/*******************************************************************************
 * Tegra scratch registers constants
 ******************************************************************************/
#define TEGRA_SCRATCH_BASE		U(0x0C390000)
#define  SECURE_SCRATCH_RSV1_LO		U(0x658)
#define  SECURE_SCRATCH_RSV1_HI		U(0x65C)
#define  SECURE_SCRATCH_RSV6		U(0x680)
#define  SECURE_SCRATCH_RSV11_LO	U(0x6A8)
#define  SECURE_SCRATCH_RSV11_HI	U(0x6AC)
#define  SECURE_SCRATCH_RSV53_LO	U(0x7F8)
#define  SECURE_SCRATCH_RSV53_HI	U(0x7FC)
#define  SECURE_SCRATCH_RSV54_HI	U(0x804)
#define  SECURE_SCRATCH_RSV55_LO	U(0x808)
#define  SECURE_SCRATCH_RSV55_HI	U(0x80C)

/*******************************************************************************
 * Tegra Memory Mapped Control Register Access constants
 ******************************************************************************/
#define TEGRA_MMCRAB_BASE		U(0x0E000000)

/*******************************************************************************
 * Tegra Memory Mapped Activity Monitor Register Access constants
 ******************************************************************************/
#define TEGRA_ARM_ACTMON_CTR_BASE	U(0x0E060000)
#define TEGRA_DENVER_ACTMON_CTR_BASE	U(0x0E070000)

/*******************************************************************************
 * Tegra SMMU Controller constants
 ******************************************************************************/
#define TEGRA_SMMU0_BASE		U(0x12000000)

/*******************************************************************************
 * Tegra TZRAM constants
 ******************************************************************************/
#define TEGRA_TZRAM_BASE		U(0x30000000)
#define TEGRA_TZRAM_SIZE		U(0x40000)

#endif /* __TEGRA_DEF_H__ */
