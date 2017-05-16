/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_DEF_H__
#define __TEGRA_DEF_H__

/*******************************************************************************
 * This value is used by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call as the `state-id` field in the 'power state' parameter.
 ******************************************************************************/
#define PSTATE_ID_SOC_POWERDN	0xD

/*******************************************************************************
 * Platform power states (used by PSCI framework)
 *
 * - PLAT_MAX_RET_STATE should be less than lowest PSTATE_ID
 * - PLAT_MAX_OFF_STATE should be greater than the highest PSTATE_ID
 ******************************************************************************/
#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		(PSTATE_ID_SOC_POWERDN + 1)

/*******************************************************************************
 * GIC memory map
 ******************************************************************************/
#define TEGRA_GICD_BASE			0x50041000
#define TEGRA_GICC_BASE			0x50042000

/*******************************************************************************
 * Tegra micro-seconds timer constants
 ******************************************************************************/
#define TEGRA_TMRUS_BASE		0x60005010
#define TEGRA_TMRUS_SIZE		0x1000

/*******************************************************************************
 * Tegra Clock and Reset Controller constants
 ******************************************************************************/
#define TEGRA_CAR_RESET_BASE		0x60006000

/*******************************************************************************
 * Tegra Flow Controller constants
 ******************************************************************************/
#define TEGRA_FLOWCTRL_BASE		0x60007000

/*******************************************************************************
 * Tegra Secure Boot Controller constants
 ******************************************************************************/
#define TEGRA_SB_BASE			0x6000C200

/*******************************************************************************
 * Tegra Exception Vectors constants
 ******************************************************************************/
#define TEGRA_EVP_BASE			0x6000F000

/*******************************************************************************
 * Tegra Miscellaneous register constants
 ******************************************************************************/
#define TEGRA_MISC_BASE			0x70000000
#define  HARDWARE_REVISION_OFFSET	0x804

/*******************************************************************************
 * Tegra UART controller base addresses
 ******************************************************************************/
#define TEGRA_UARTA_BASE		0x70006000
#define TEGRA_UARTB_BASE		0x70006040
#define TEGRA_UARTC_BASE		0x70006200
#define TEGRA_UARTD_BASE		0x70006300
#define TEGRA_UARTE_BASE		0x70006400

/*******************************************************************************
 * Tegra Power Mgmt Controller constants
 ******************************************************************************/
#define TEGRA_PMC_BASE			0x7000E400

/*******************************************************************************
 * Tegra Memory Controller constants
 ******************************************************************************/
#define TEGRA_MC_BASE			0x70019000

/* TZDRAM carveout configuration registers */
#define MC_SECURITY_CFG0_0		0x70
#define MC_SECURITY_CFG1_0		0x74
#define MC_SECURITY_CFG3_0		0x9BC

/* Video Memory carveout configuration registers */
#define MC_VIDEO_PROTECT_BASE_HI	0x978
#define MC_VIDEO_PROTECT_BASE_LO	0x648
#define MC_VIDEO_PROTECT_SIZE_MB	0x64c

/*******************************************************************************
 * Tegra TZRAM constants
 ******************************************************************************/
#define TEGRA_TZRAM_BASE		0x7C010000
#define TEGRA_TZRAM_SIZE		0x10000

#endif /* __TEGRA_DEF_H__ */
