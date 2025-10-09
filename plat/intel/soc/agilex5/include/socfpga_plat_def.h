/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SOCFPGA_DEF_H
#define PLAT_SOCFPGA_DEF_H

#include "agilex5_memory_controller.h"
#include "agilex5_system_manager.h"

#include <platform_def.h>

/* Platform Setting */
#define PLATFORM_MODEL						PLAT_SOCFPGA_AGILEX5
/* 1 = Flush cache, 0 = No cache flush.
 * Default for Agilex5 is Cache flush.
 */
#define CACHE_FLUSH							1
#define MMC_DEVICE_TYPE						1  /* MMC = 0, SD = 1 */
#define XLAT_TABLES_V2						U(1)
#define PLAT_PRIMARY_CPU_A55					0x000
#define PLAT_PRIMARY_CPU_A76					0x200
#define PLAT_CLUSTER_ID_MPIDR_AFF_SHIFT				MPIDR_AFF2_SHIFT
#define PLAT_CPU_ID_MPIDR_AFF_SHIFT				MPIDR_AFF1_SHIFT
#define PLAT_L2_RESET_REQ					0xB007C0DE
#define PLAT_HANDOFF_OFFSET					0x0007F000
#define PLAT_TIMER_BASE_ADDR					0x10D01000
#define SOCFPGA_DTB_BASE			0x80020000
#define DT_COMPATIBLE_STR			"arm,altera socfpga-agilex5"

/* System Counter */
/* TODO: Update back to 400MHz.
 * This shall be updated to read from L4 clock instead of hardcoded.
 */
#define PLAT_SYS_COUNTER_FREQ_IN_TICKS				U(400000000)
#define PLAT_SYS_COUNTER_FREQ_IN_MHZ				U(400)

/* FPGA config helpers */
#define INTEL_SIP_SMC_FPGA_CONFIG_ADDR				0x80400000
#define INTEL_SIP_SMC_FPGA_CONFIG_SIZE				0x82000000

/* QSPI Setting */
#define CAD_QSPIDATA_OFST					0x10900000
#define CAD_QSPI_OFFSET						0x108d2000

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

/* Register Mapping */
#define SOCFPGA_CCU_NOC_REG_BASE				0x1c000000
#define SOCFPGA_F2SDRAMMGR_REG_BASE				0x18001000

#define SOCFPGA_MMC_REG_BASE					0x10808000
#define SOCFPGA_MEMCTRL_REG_BASE				0x108CC000
#define SOCFPGA_RSTMGR_REG_BASE					0x10d11000
#define SOCFPGA_SYSMGR_REG_BASE					0x10d12000
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

//128MB PSS
#define PSS_BASE						(0x10000000)
#define PSS_SIZE						(0x08000000)

//64MB MPFE
#define MPFE_BASE						(0x18000000)
#define MPFE_SIZE						(0x04000000)

//16MB CCU
#define CCU_BASE						(0x1C000000)
#define CCU_SIZE						(0x01000000)

//1MB GIC
#define GIC_BASE						(0x1D000000)
#define GIC_SIZE						(0x00100000)

#define BL2_BASE						(0x00000000)
#define BL2_LIMIT						(0x0007E000)

#define BL31_BASE						(0x80000000)
#define BL31_LIMIT						(0x82000000)
/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define PLAT_UART0_BASE						(0x10C02000)
#define PLAT_UART1_BASE						(0x10C02100)

/*******************************************************************************
 * WDT related constants
 ******************************************************************************/
#define WDT_BASE						(0x10D00200)

/*******************************************************************************
 * GIC related constants
 ******************************************************************************/
#define PLAT_GIC_BASE						(0x1D000000)
#define PLAT_GICC_BASE						(PLAT_GIC_BASE + 0x20000)
#define PLAT_GICD_BASE						(PLAT_GIC_BASE + 0x00000)
#define PLAT_GICR_BASE						(PLAT_GIC_BASE + 0x60000)

#define PLAT_INTEL_SOCFPGA_GICR_BASE				PLAT_GICR_BASE

/*******************************************************************************
 * SDMMC related pointer function
 ******************************************************************************/
#define SDMMC_READ_BLOCKS					sdmmc_read_blocks
#define SDMMC_WRITE_BLOCKS					sdmmc_write_blocks

/*******************************************************************************
 * sysmgr.boot_scratch_cold3 bits[5:2] are used to indicate L2 reset
 * is done, or SMP secondary cores boot request status.
 ******************************************************************************/
#define L2_RESET_DONE_REG					SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_3)

#endif /* PLAT_SOCFPGA_DEF_H */
