/*
 * Copyright (c) 2019-2026, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SOCFPGA_DEF_H
#define PLAT_SOCFPGA_DEF_H

#include "agilex72_cache.h"
#include "agilex72_memory_controller.h"
#include "agilex72_power_manager.h"
#include "agilex72_system_manager.h"
#include "mcsr/socfpga_mcsr_mailbox.h"
#include <platform_def.h>

/* MCSR mailbox backend (pulls in C2S/S2C ring helpers + backend vtable). */

/* Platform Setting */
#define PLATFORM_MODEL						PLAT_SOCFPGA_AGILEX72
/* 1 = Flush cache, 0 = No cache flush.
 * Default for Agilex72 is Cache flush.
 */
#define CACHE_FLUSH							1
#define MMC_DEVICE_TYPE						1  /* MMC = 0, SD = 1 */
#define XLAT_TABLES_V2						U(1)
#define PLAT_PRIMARY_CPU_A520_0				U(0x0)
#define PLAT_PRIMARY_CPU_A520_1				U(0x100)
#define PLAT_PRIMARY_CPU_A720_2				U(0x200)
#define PLAT_PRIMARY_CPU_A720_3				U(0x300)
#define PLAT_PRIMARY_CPU					PLAT_PRIMARY_CPU_A520_0
#define PLAT_CLUSTER_ID_MPIDR_AFF_SHIFT				MPIDR_AFF2_SHIFT
#define PLAT_CPU_ID_MPIDR_AFF_SHIFT				MPIDR_AFF1_SHIFT
#define PLAT_L2_RESET_REQ					0xB007C0DE
#define PLAT_HANDOFF_OFFSET					0x0007F000
#define PLAT_TIMER_BASE_ADDR					0x09064000
#define SOCFPGA_DTB_BASE			0x80020000
#define DT_COMPATIBLE_STR			"arm,altera socfpga-km"

/*
 * System counter frequency. This will eventually be read from the L4 clock
 * instead of being hard-coded.
 */
#define PLAT_SYS_COUNTER_FREQ_IN_TICKS				U(400000000)
#define PLAT_SYS_COUNTER_FREQ_IN_MHZ				U(400)

/* FPGA config helpers */
#define INTEL_SIP_SMC_FPGA_CONFIG_ADDR				0x80400000
#define INTEL_SIP_SMC_FPGA_CONFIG_SIZE				0x82000000

/* QSPI Setting */
#define CAD_QSPIDATA_OFST					0x10900000
#define CAD_QSPI_OFFSET						0x108d2000

/* XSPI/OSPI Setting - Based on Simics memory map */
#define CDNS_XSPI_OFFSET					0x09008000  /* ospi0 controller */
#define CDNS_XSPI_SDMA_OFFSET				0x04000000  /* ospi_sdma0 */
#define CDNS_XSPI_AUX_OFFSET				0x0900A000  /* ospi_combophy_reg0 */
#define CDNS_XSPI_SDMA_SIZE					0x2000000   /* 32MB SDMA region */
/* xSPI flash data staging area in DDR for Simics testing. */
#define CDNS_XSPIDATA_OFST					0xB0000000
#define PLAT_XSPI_DATA_BASE					CDNS_XSPIDATA_OFST

/* FIP Setting */
#define PLAT_FIP_BASE						(0)
#if ARM_LINUX_KERNEL_AS_BL33
#define PLAT_FIP_MAX_SIZE					(0x8000000)
#else
#define PLAT_FIP_MAX_SIZE					(0x1000000)
#endif

/* SDMMC Setting */
#if ARM_LINUX_KERNEL_AS_BL33
#define PLAT_MMC_DATA_BASE					(0x90000000)
#define PLAT_MMC_DATA_SIZE					(0x100000)
#define SOCFPGA_MMC_BLOCK_SIZE					U(32768)
#else
#define PLAT_MMC_DATA_BASE					(0x0007D000)
#define PLAT_MMC_DATA_SIZE					(0x2000)
#define SOCFPGA_MMC_BLOCK_SIZE					U(8192)
#endif

#define PLAT_NAND_SCRATCH_BUFF					(0x96400000)

/*
 * MCSR SDM mailbox — interleaved per-client layout.
 * Each client n occupies a 16 KB (0x4000) window from MCSR_BASE:
 *   n*0x4000 + 0x0000  C2S RO   (SDM writes STATUS/COUT; client reads)
 *   n*0x4000 + 0x1000  C2S RW   (client writes CTRL/DESC/CIN/RESET; last 4 regs trigger SDM intr)
 *   n*0x4000 + 0x2000  S2C RO   (client writes COUT; SDM reads)
 *   n*0x4000 + 0x3000  S2C RW   (SDM writes CTRL/DESC/CIN; last 4 regs trigger client intr)
 * Client 0 (ATF/HPS):
 */
#define SOCFPGA_SDM_MCSR_C2S_0_R0	0x18100000	/* C2S RO: +0x0000 */
#define SOCFPGA_SDM_MCSR_C2S_0_RW	0x18101000	/* C2S RW: +0x1000 */
#define SOCFPGA_SDM_MCSR_S2C_0_RO	0x18102000	/* S2C RO: +0x2000 */
#define SOCFPGA_SDM_MCSR_S2C_0_RW	0x18103000	/* S2C RW: +0x3000 */

#define SOCFPGA_MAILBOX_C2S_BASE		SOCFPGA_SDM_MCSR_C2S_0_R0
#define SOCFPGA_MAILBOX_C2S_RINGBUF_BASE	SOCFPGA_SDM_MCSR_C2S_0_RW
#define SOCFPGA_MAILBOX_S2C_BASE		SOCFPGA_SDM_MCSR_S2C_0_RO
#define SOCFPGA_MAILBOX_S2C_RINGBUF_BASE	SOCFPGA_SDM_MCSR_S2C_0_RW

/* Register Mapping */
#define SOCFPGA_CCU_NOC_REG_BASE				0x06000000

#define SOCFPGA_F2SDRAMMGR_REG_BASE				0x18001000

#define SOCFPGA_MMC_REG_BASE					0x09000000
#define SOCFPGA_MEMCTRL_REG_BASE				0x108CC000
#define SOCFPGA_RSTMGR_REG_BASE					0x09161000
#define SOCFPGA_SYSMGR_REG_BASE					0x0d2b8000
#define SOCFPGA_HS_SYSMGR_REG_BASE				0x0d2b0000
#define SOCFPGA_LS_SYSMGR_REG_BASE				0x0a968000
#define SOCFPGA_PINMUX_REG_BASE					0x10d13000
#define SOCFPGA_NAND_REG_BASE					0x10B80000
#define SOCFPGA_ECC_QSPI_REG_BASE				0x10A22000

#define SOCFPGA_L4_PER_SCR_REG_BASE				0x10d21000
#define SOCFPGA_L4_SYS_SCR_REG_BASE				0x10d21100
#define SOCFPGA_SOC2FPGA_SCR_REG_BASE				0x10d21200
#define SOCFPGA_LWSOC2FPGA_SCR_REG_BASE				0x10d21300
#define SOCFPGA_SDMMC_SECU_BIT					0x40
#define SOCFPGA_LWSOC2FPGA_ENABLE				0xffe0301
#define SOCFPGA_SDMMC_SECU_BIT_ENABLE				0x1010001

/* Define maximum page size for NAND flash devices */
#define PLATFORM_MTD_MAX_PAGE_SIZE				U(0x2000)

/* OCRAM Register*/

#define OCRAM_REG_BASE						0x108CC400
#define OCRAM_REGION_0_OFFSET					0x18
#define OCRAM_REGION_0_REG_BASE					(OCRAM_REG_BASE + \
								OCRAM_REGION_0_OFFSET)
#define OCRAM_NON_SECURE_ENABLE					0x0

/*
 * Magic key bits: 4 bits[5:2] from boot scratch register COLD3 are used to
 * indicate the below requests/status
 *     0x0       : Default value on reset, not used
 *     0x1       : L2/warm reset is completed
 *     0x2       : SMP secondary core boot requests
 *     0x3 - 0xF : Reserved for future use
 */
#define BS_REG_MAGIC_KEYS_MASK			0x3C
#define BS_REG_MAGIC_KEYS_POS			0x02
#define L2_RESET_DONE_STATUS			(0x01 << BS_REG_MAGIC_KEYS_POS)
#define SMP_SEC_CORE_BOOT_REQ			(0x02 << BS_REG_MAGIC_KEYS_POS)
#define ALIGN_CHECK_64BIT_MASK			0x07

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define DRAM_BASE						(0x80000000)
#define DRAM_SIZE						(0x80000000)

#define OCRAM_BASE						(0x00000000)
#define OCRAM_SIZE						(0x00080000)

#define MEM64_BASE						(0x0080000000)
#define MEM64_SIZE						(0x0080000000)

/* 128 MB PSS */
#define PSS_BASE						(0x10000000)
#define PSS_SIZE						(0x08000000)

/* 64 MB MPFE */
#define MPFE_BASE						(0x18000000)
#define MPFE_SIZE						(0x04000000)

/* 16 MB CCU */
#define CCU_BASE						(0x1C000000)
#define CCU_SIZE						(0x01000000)

/* 1 MB GIC */
#define GIC_BASE						(0x07000000)
#define GIC_SIZE						(0x00200000)

#define BL2_BASE						(0x00000000)
#define BL2_LIMIT						(0x0007E000)

#define BL31_BASE						(0x80000000)
#define BL31_LIMIT						(0x82000000)
/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define PLAT_UART0_BASE						(0x09038000)
#define PLAT_UART1_BASE						(0x10C02100)

/*******************************************************************************
 * WDT related constants
 ******************************************************************************/
#define WDT_BASE						(0x9078000)

/*******************************************************************************
 * GIC related constants
 ******************************************************************************/
#define PLAT_GIC_BASE						(0x7000000)
#define PLAT_GICC_BASE						(PLAT_GIC_BASE + 0x20000)
#define PLAT_GICD_BASE						(PLAT_GIC_BASE + 0x00000)
#define PLAT_GICR_BASE						(PLAT_GIC_BASE + 0x80000)

#define PLAT_INTEL_SOCFPGA_GICR_BASE		PLAT_GICR_BASE
/* Aliases required by drivers/arm/gic/v3/gicv3_base.c (USE_GIC_DRIVER=3) */
#define PLAT_ARM_GICD_BASE				PLAT_GICD_BASE
#define PLAT_ARM_GICR_BASE				PLAT_GICR_BASE


/*******************************************************************************
 * SDMMC related pointer function
 ******************************************************************************/
#define SDMMC_READ_BLOCKS					sdmmc_read_blocks
#define SDMMC_WRITE_BLOCKS					sdmmc_write_blocks

/*******************************************************************************
 * Power Manager related constants
 ******************************************************************************/
#define PWRMGR_MPU_PCHCTLR					AGX72_PWRMGR(MPU_PCHCTLR)
#define PWRMGR_CPU_POWER_STATE_MASK			AGX72_PWRMGR_CPU_POWER_STATE_MASK
#define PWRMGR_MPU_TRIGGER_PCH_CPU			AGX72_PWRMGR_MPU_TRIGGER_PCH_CPU

/*******************************************************************************
 * sysmgr.boot_scratch_cold3 bits[5:2] are used to indicate L2 reset
 * is done, or SMP secondary cores boot request status.
 ******************************************************************************/
#define L2_RESET_DONE_REG					SOCFPGA_SYSMGR(BOOT_SCRATCH_POR_3)

/* Reset manager stores the BL31 entry shifted by 2 bits */
#define CPU_RESETBASELOW_CPU_SHIFT			U(2)

#endif /* PLAT_SOCFPGA_DEF_H */
