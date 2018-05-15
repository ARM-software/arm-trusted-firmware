/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_DEF_H
#define TEGRA_DEF_H

#include <lib/utils_def.h>

/*******************************************************************************
 * This value is used by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call as the `state-id` field in the 'power state' parameter.
 ******************************************************************************/
#define PSTATE_ID_SOC_POWERDN	U(0xD)

/*******************************************************************************
 * Platform power states (used by PSCI framework)
 *
 * - PLAT_MAX_RET_STATE should be less than lowest PSTATE_ID
 * - PLAT_MAX_OFF_STATE should be greater than the highest PSTATE_ID
 ******************************************************************************/
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		(PSTATE_ID_SOC_POWERDN + U(1))

/*******************************************************************************
 * Chip specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 35)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 35)

/*******************************************************************************
 * GIC memory map
 ******************************************************************************/
#define TEGRA_GICD_BASE			U(0x50041000)
#define TEGRA_GICC_BASE			U(0x50042000)

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
#define TEGRA_GPU_RESET_GPU_SET_OFFSET	U(0x290)
#define  GPU_RESET_BIT			(U(1) << 24)
#define  GPU_SET_BIT			(U(1) << 24)

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

/* Memory Controller Interrupt Status */
#define MC_INTSTATUS			0x00U

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

/*******************************************************************************
 * Tegra DRAM memory base address
 ******************************************************************************/
#define TEGRA_DRAM_BASE			ULL(0x80000000)
#define TEGRA_DRAM_END			ULL(0x27FFFFFFF)

#endif /* TEGRA_DEF_H */
