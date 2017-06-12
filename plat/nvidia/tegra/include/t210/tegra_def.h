/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_DEF_H
#define TEGRA_DEF_H

#include <lib/utils_def.h>

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
 * iRAM memory constants
 ******************************************************************************/
#define TEGRA_IRAM_BASE			0x40000000

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
 * Tegra Resource Semaphore constants
 ******************************************************************************/
#define TEGRA_RES_SEMA_BASE		0x60001000UL
#define  STA_OFFSET			0UL
#define  SET_OFFSET			4UL
#define  CLR_OFFSET			8UL

/*******************************************************************************
 * Tegra Primary Interrupt Controller constants
 ******************************************************************************/
#define TEGRA_PRI_ICTLR_BASE		0x60004000UL
#define  CPU_IEP_FIR_SET		0x18UL

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
#define TEGRA_RST_DEV_CLR_V		U(0x434)
#define TEGRA_CLK_ENB_V			U(0x440)

/*******************************************************************************
 * Tegra Flow Controller constants
 ******************************************************************************/
#define TEGRA_FLOWCTRL_BASE		U(0x60007000)

/*******************************************************************************
 * Tegra AHB arbitration controller
 ******************************************************************************/
#define TEGRA_AHB_ARB_BASE		0x6000C000UL

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
 * Tegra Atomics constants
 ******************************************************************************/
#define TEGRA_ATOMICS_BASE		0x70016000UL
#define  TRIGGER0_REG_OFFSET		0UL
#define  TRIGGER_WIDTH_SHIFT		4UL
#define  TRIGGER_ID_SHIFT		16UL
#define  RESULT0_REG_OFFSET		0xC00UL

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
 * Tegra SE constants
 ******************************************************************************/
#define TEGRA_SE1_BASE			U(0x70012000)
#define TEGRA_SE2_BASE			U(0x70412000)
#define TEGRA_PKA1_BASE			U(0x70420000)
#define TEGRA_SE2_RANGE_SIZE		U(0x2000)
#define SE_TZRAM_SECURITY		U(0x4)

/*******************************************************************************
 * Tegra TZRAM constants
 ******************************************************************************/
#define TEGRA_TZRAM_BASE		U(0x7C010000)
#define TEGRA_TZRAM_SIZE		U(0x10000)

#endif /* TEGRA_DEF_H */
