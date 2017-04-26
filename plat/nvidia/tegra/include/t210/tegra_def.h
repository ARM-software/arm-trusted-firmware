/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_DEF_H__
#define __TEGRA_DEF_H__

#include <utils_def.h>

/*******************************************************************************
 * Power down state IDs
 ******************************************************************************/
#define PSTATE_ID_CORE_POWERDN		U(7)
#define PSTATE_ID_CLUSTER_IDLE		U(16)
#define PSTATE_ID_CLUSTER_POWERDN	U(17)
#define PSTATE_ID_SOC_POWERDN		U(27)

/*******************************************************************************
 * This value is used by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call as the `state-id` field in the 'power state' parameter.
 ******************************************************************************/
#define PLAT_SYS_SUSPEND_STATE_ID	PSTATE_ID_SOC_POWERDN

/*******************************************************************************
 * Platform power states (used by PSCI framework)
 *
 * - PLAT_MAX_RET_STATE should be less than lowest PSTATE_ID
 * - PLAT_MAX_OFF_STATE should be greater than the highest PSTATE_ID
 ******************************************************************************/
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		(PSTATE_ID_SOC_POWERDN + U(1))

/*******************************************************************************
 * GIC memory map
 ******************************************************************************/
#define TEGRA_GICD_BASE			U(0x50041000)
#define TEGRA_GICC_BASE			U(0x50042000)

/*******************************************************************************
 * Tegra Memory Select Switch Controller constants
 ******************************************************************************/
#define TEGRA_MSELECT_BASE		U(0x50060000)

#define MSELECT_CONFIG			U(0x0)
#define ENABLE_WRAP_INCR_MASTER2_BIT	(U(1) << U(29))
#define ENABLE_WRAP_INCR_MASTER1_BIT	(U(1) << U(28))
#define ENABLE_WRAP_INCR_MASTER0_BIT	(U(1) << U(27))
#define UNSUPPORTED_TX_ERR_MASTER2_BIT	(U(1) << U(25))
#define UNSUPPORTED_TX_ERR_MASTER1_BIT	(U(1) << U(24))
#define ENABLE_UNSUP_TX_ERRORS		(UNSUPPORTED_TX_ERR_MASTER2_BIT | \
					 UNSUPPORTED_TX_ERR_MASTER1_BIT)
#define ENABLE_WRAP_TO_INCR_BURSTS	(ENABLE_WRAP_INCR_MASTER2_BIT | \
					 ENABLE_WRAP_INCR_MASTER1_BIT | \
					 ENABLE_WRAP_INCR_MASTER0_BIT)

/*******************************************************************************
 * Tegra micro-seconds timer constants
 ******************************************************************************/
#define TEGRA_TMRUS_BASE		U(0x60005010)
#define TEGRA_TMRUS_SIZE		U(0x1000)

/*******************************************************************************
 * Tegra Clock and Reset Controller constants
 ******************************************************************************/
#define TEGRA_CAR_RESET_BASE		U(0x60006000)
#define TEGRA_GPU_RESET_REG_OFFSET	U(0x28C)
#define  GPU_RESET_BIT			(U(1) << 24)

/*******************************************************************************
 * Tegra Flow Controller constants
 ******************************************************************************/
#define TEGRA_FLOWCTRL_BASE		U(0x60007000)

/*******************************************************************************
 * Tegra Secure Boot Controller constants
 ******************************************************************************/
#define TEGRA_SB_BASE			U(0x6000C200)

/*******************************************************************************
 * Tegra Exception Vectors constants
 ******************************************************************************/
#define TEGRA_EVP_BASE			U(0x6000F000)

/*******************************************************************************
 * Tegra Miscellaneous register constants
 ******************************************************************************/
#define TEGRA_MISC_BASE			U(0x70000000)
#define  HARDWARE_REVISION_OFFSET	U(0x804)

/*******************************************************************************
 * Tegra UART controller base addresses
 ******************************************************************************/
#define TEGRA_UARTA_BASE		U(0x70006000)
#define TEGRA_UARTB_BASE		U(0x70006040)
#define TEGRA_UARTC_BASE		U(0x70006200)
#define TEGRA_UARTD_BASE		U(0x70006300)
#define TEGRA_UARTE_BASE		U(0x70006400)

/*******************************************************************************
 * Tegra Power Mgmt Controller constants
 ******************************************************************************/
#define TEGRA_PMC_BASE			U(0x7000E400)

/*******************************************************************************
 * Tegra Memory Controller constants
 ******************************************************************************/
#define TEGRA_MC_BASE			U(0x70019000)

/* TZDRAM carveout configuration registers */
#define MC_SECURITY_CFG0_0		U(0x70)
#define MC_SECURITY_CFG1_0		U(0x74)
#define MC_SECURITY_CFG3_0		U(0x9BC)

/* Video Memory carveout configuration registers */
#define MC_VIDEO_PROTECT_BASE_HI	U(0x978)
#define MC_VIDEO_PROTECT_BASE_LO	U(0x648)
#define MC_VIDEO_PROTECT_SIZE_MB	U(0x64c)

/*******************************************************************************
 * Tegra TZRAM constants
 ******************************************************************************/
#define TEGRA_TZRAM_BASE		U(0x7C010000)
#define TEGRA_TZRAM_SIZE		U(0x10000)

#endif /* __TEGRA_DEF_H__ */
