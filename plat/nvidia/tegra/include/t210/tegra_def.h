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
 * Power down state IDs
 ******************************************************************************/
#define PSTATE_ID_CORE_POWERDN		U(7)
#define PSTATE_ID_CLUSTER_IDLE		U(16)
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
 * Chip specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 35)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 35)

/*******************************************************************************
 * SC7 entry firmware's header size
 ******************************************************************************/
#define SC7ENTRY_FW_HEADER_SIZE_BYTES	U(0x400)

/*******************************************************************************
 * iRAM memory constants
 ******************************************************************************/
#define TEGRA_IRAM_BASE			U(0x40000000)
#define TEGRA_IRAM_A_SIZE		U(0x10000) /* 64KB */
#define TEGRA_IRAM_SIZE			U(40000) /* 256KB */

/*******************************************************************************
 * GIC memory map
 ******************************************************************************/
#define TEGRA_GICD_BASE			U(0x50041000)
#define TEGRA_GICC_BASE			U(0x50042000)

/*******************************************************************************
 * Secure IRQ definitions
 ******************************************************************************/
#define TEGRA210_WDT_CPU_LEGACY_FIQ		U(28)

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
#define TEGRA_BOND_OUT_H		U(0x74)
#define  APB_DMA_LOCK_BIT		(U(1) << 2)
#define  AHB_DMA_LOCK_BIT		(U(1) << 1)
#define TEGRA_BOND_OUT_U		U(0x78)
#define  IRAM_D_LOCK_BIT		(U(1) << 23)
#define  IRAM_C_LOCK_BIT		(U(1) << 22)
#define  IRAM_B_LOCK_BIT		(U(1) << 21)
#define TEGRA_GPU_RESET_REG_OFFSET	U(0x28C)
#define TEGRA_GPU_RESET_GPU_SET_OFFSET	U(0x290)
#define  GPU_RESET_BIT			(U(1) << 24)
#define  GPU_SET_BIT			(U(1) << 24)
#define TEGRA_RST_DEV_SET_Y		U(0x2a8)
#define  NVENC_RESET_BIT		(U(1) << 27)
#define  TSECB_RESET_BIT		(U(1) << 14)
#define  APE_RESET_BIT			(U(1) << 6)
#define  NVJPG_RESET_BIT		(U(1) << 3)
#define  NVDEC_RESET_BIT		(U(1) << 2)
#define TEGRA_RST_DEV_SET_L		U(0x300)
#define  HOST1X_RESET_BIT		(U(1) << 28)
#define  ISP_RESET_BIT			(U(1) << 23)
#define  USBD_RESET_BIT			(U(1) << 22)
#define  VI_RESET_BIT			(U(1) << 20)
#define  SDMMC4_RESET_BIT		(U(1) << 15)
#define  SDMMC1_RESET_BIT		(U(1) << 14)
#define  SDMMC2_RESET_BIT		(U(1) << 9)
#define TEGRA_RST_DEV_SET_H		U(0x308)
#define  USB2_RESET_BIT			(U(1) << 26)
#define  APBDMA_RESET_BIT		(U(1) << 2)
#define  AHBDMA_RESET_BIT		(U(1) << 1)
#define TEGRA_RST_DEV_SET_U		U(0x310)
#define  XUSB_DEV_RESET_BIT		(U(1) << 31)
#define  XUSB_HOST_RESET_BIT		(U(1) << 25)
#define  TSEC_RESET_BIT			(U(1) << 19)
#define  PCIE_RESET_BIT			(U(1) << 6)
#define  SDMMC3_RESET_BIT		(U(1) << 5)
#define TEGRA_RST_DEVICES_V		U(0x358)
#define TEGRA_RST_DEVICES_W		U(0x35C)
#define  ENTROPY_CLK_ENB_BIT		(U(1) << 21)
#define TEGRA_CLK_OUT_ENB_V		U(0x360)
#define  SE_CLK_ENB_BIT			(U(1) << 31)
#define TEGRA_CLK_OUT_ENB_W		U(0x364)
#define  ENTROPY_RESET_BIT 		(U(1) << 21)
#define TEGRA_RST_DEV_SET_V		U(0x430)
#define  SE_RESET_BIT			(U(1) << 31)
#define  HDA_RESET_BIT			(U(1) << 29)
#define  SATA_RESET_BIT			(U(1) << 28)
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
#define  APB_SLAVE_SECURITY_ENABLE	U(0xC00)
#define  PMC_SECURITY_EN_BIT		(U(1) << 13)
#define  PINMUX_AUX_DVFS_PWM		U(0x3184)
#define  PINMUX_PWM_TRISTATE		(U(1) << 4)

/*******************************************************************************
 * Tegra UART controller base addresses
 ******************************************************************************/
#define TEGRA_UARTA_BASE		U(0x70006000)
#define TEGRA_UARTB_BASE		U(0x70006040)
#define TEGRA_UARTC_BASE		U(0x70006200)
#define TEGRA_UARTD_BASE		U(0x70006300)
#define TEGRA_UARTE_BASE		U(0x70006400)

/*******************************************************************************
 * Tegra Fuse Controller related constants
 ******************************************************************************/
#define TEGRA_FUSE_BASE			0x7000F800UL
#define FUSE_BOOT_SECURITY_INFO		0x268UL
#define FUSE_ATOMIC_SAVE_CARVEOUT_EN	(0x1U << 7)
#define FUSE_JTAG_SECUREID_VALID	(0x104UL)
#define ECID_VALID			(0x1UL)


/*******************************************************************************
 * Tegra Power Mgmt Controller constants
 ******************************************************************************/
#define TEGRA_PMC_BASE			U(0x7000E400)
#define TEGRA_PMC_SIZE			U(0xC00) /* 3k */

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

/* SMMU configuration registers*/
#define MC_SMMU_PPCS_ASID_0		0x270U
#define  PPCS_SMMU_ENABLE		(0x1U << 31)

/*******************************************************************************
 * Tegra CLDVFS constants
 ******************************************************************************/
#define TEGRA_CL_DVFS_BASE		U(0x70110000)
#define DVFS_DFLL_CTRL			U(0x00)
#define  ENABLE_OPEN_LOOP		U(1)
#define  ENABLE_CLOSED_LOOP		U(2)
#define DVFS_DFLL_OUTPUT_CFG		U(0x20)
#define  DFLL_OUTPUT_CFG_I2C_EN_BIT	(U(1) << 30)
#define  DFLL_OUTPUT_CFG_CLK_EN_BIT	(U(1) << 6)

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

/*******************************************************************************
 * Tegra TZRAM carveout constants
 ******************************************************************************/
#define TEGRA_TZRAM_CARVEOUT_BASE	U(0x7C04C000)
#define TEGRA_TZRAM_CARVEOUT_SIZE	U(0x4000)

/*******************************************************************************
 * Tegra DRAM memory base address
 ******************************************************************************/
#define TEGRA_DRAM_BASE			ULL(0x80000000)
#define TEGRA_DRAM_END			ULL(0x27FFFFFFF)

#endif /* TEGRA_DEF_H */
