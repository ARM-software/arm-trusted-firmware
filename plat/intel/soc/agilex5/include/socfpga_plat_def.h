/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
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
#define BOOT_SOURCE						BOOT_SOURCE_SDMMC
#define MMC_DEVICE_TYPE						1  /* MMC = 0, SD = 1 */
#define XLAT_TABLES_V2						U(1)
#define PLAT_PRIMARY_CPU_A55					0x000
#define PLAT_PRIMARY_CPU_A76					0x200
#define PLAT_CLUSTER_ID_MPIDR_AFF_SHIFT				MPIDR_AFF2_SHIFT
#define PLAT_CPU_ID_MPIDR_AFF_SHIFT				MPIDR_AFF1_SHIFT
#define PLAT_L2_RESET_REQ			0xB007C0DE

/* System Counter */ /* TODO: Update back to 400MHz */
#define PLAT_SYS_COUNTER_FREQ_IN_TICKS				(80000000)
#define PLAT_SYS_COUNTER_FREQ_IN_MHZ				(80)

/* FPGA config helpers */
#define INTEL_SIP_SMC_FPGA_CONFIG_ADDR				0x400000
#define INTEL_SIP_SMC_FPGA_CONFIG_SIZE				0x2000000

/* QSPI Setting */
#define CAD_QSPIDATA_OFST					0x10900000
#define CAD_QSPI_OFFSET						0x108d2000

/* Register Mapping */
#define SOCFPGA_CCU_NOC_REG_BASE				0x1c000000
#define SOCFPGA_F2SDRAMMGR_REG_BASE				0x18001000

#define SOCFPGA_MMC_REG_BASE					0x10808000
#define SOCFPGA_MEMCTRL_REG_BASE				0x108CC000
#define SOCFPGA_RSTMGR_REG_BASE					0x10d11000
#define SOCFPGA_SYSMGR_REG_BASE					0x10d12000
#define SOCFPGA_PINMUX_REG_BASE					0x10d13000
#define SOCFPGA_NAND_REG_BASE					0x10B80000

#define SOCFPGA_L4_PER_SCR_REG_BASE				0x10d21000
#define SOCFPGA_L4_SYS_SCR_REG_BASE				0x10d21100
#define SOCFPGA_SOC2FPGA_SCR_REG_BASE				0x10d21200
#define SOCFPGA_LWSOC2FPGA_SCR_REG_BASE				0x10d21300

/* Define maximum page size for NAND flash devices */
#define PLATFORM_MTD_MAX_PAGE_SIZE				U(0x1000)

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
#define BL2_LIMIT						(0x0001b000)

#define BL31_BASE						(0x80000000)
#define BL31_LIMIT						(0x82000000)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define PLAT_UART0_BASE						(0x10C02000)
#define PLAT_UART1_BASE						(0x10C02100)

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
#define SDMMC_READ_BLOCKS	sdmmc_read_blocks
#define SDMMC_WRITE_BLOCKS	sdmmc_write_blocks

/*******************************************************************************
 * sysmgr.boot_scratch_cold6 & 7 (64bit) are used to indicate L2 reset
 * is done and HPS should trigger warm reset via RMR_EL3.
 ******************************************************************************/
#define L2_RESET_DONE_REG			0x10D12218

#endif /* PLAT_SOCFPGA_DEF_H */
