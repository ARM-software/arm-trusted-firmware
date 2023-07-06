/*
 * Copyright (c) 2019-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SOCFPGA_DEF_H
#define PLAT_SOCFPGA_DEF_H

#include <platform_def.h>
#include "s10_system_manager.h"

/* Platform Setting */
#define PLATFORM_MODEL						PLAT_SOCFPGA_STRATIX10
#define BOOT_SOURCE							BOOT_SOURCE_SDMMC
#define PLAT_PRIMARY_CPU					0
#define PLAT_CLUSTER_ID_MPIDR_AFF_SHIFT		MPIDR_AFF1_SHIFT
#define PLAT_CPU_ID_MPIDR_AFF_SHIFT			MPIDR_AFF0_SHIFT

/* FPGA config helpers */
#define INTEL_SIP_SMC_FPGA_CONFIG_ADDR		0x400000
#define INTEL_SIP_SMC_FPGA_CONFIG_SIZE		0x1000000

/* QSPI Setting */
#define CAD_QSPIDATA_OFST			0xff900000
#define CAD_QSPI_OFFSET				0xff8d2000

/* Register Mapping */
#define SOCFPGA_CCU_NOC_REG_BASE		0xf7000000
#define SOCFPGA_F2SDRAMMGR_REG_BASE		U(0xf8024000)

#define SOCFPGA_MMC_REG_BASE                    0xff808000

#define SOCFPGA_RSTMGR_REG_BASE			0xffd11000
#define SOCFPGA_SYSMGR_REG_BASE			0xffd12000

#define SOCFPGA_L4_PER_SCR_REG_BASE		0xffd21000
#define SOCFPGA_L4_SYS_SCR_REG_BASE		0xffd21100
#define SOCFPGA_SOC2FPGA_SCR_REG_BASE		0xffd21200
#define SOCFPGA_LWSOC2FPGA_SCR_REG_BASE		0xffd21300

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define DRAM_BASE				(0x0)
#define DRAM_SIZE				(0x80000000)

#define OCRAM_BASE				(0xFFE00000)
#define OCRAM_SIZE				(0x00040000)

#define MEM64_BASE				(0x0100000000)
#define MEM64_SIZE				(0x1F00000000)

#define DEVICE1_BASE				(0x80000000)
#define DEVICE1_SIZE				(0x60000000)

#define DEVICE2_BASE				(0xF7000000)
#define DEVICE2_SIZE				(0x08E00000)

#define DEVICE3_BASE				(0xFFFC0000)
#define DEVICE3_SIZE				(0x00008000)

#define DEVICE4_BASE				(0x2000000000)
#define DEVICE4_SIZE				(0x0100000000)

#define BL2_BASE		(0xffe00000)
#define BL2_LIMIT		(0xffe1b000)

#define BL31_BASE		(0x1000)
#define BL31_LIMIT		(0x81000)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define PLAT_UART0_BASE		(0xFFC02000)
#define PLAT_UART1_BASE		(0xFFC02100)

/*******************************************************************************
 * GIC related constants
 ******************************************************************************/
#define PLAT_GIC_BASE			(0xFFFC0000)
#define PLAT_GICC_BASE			(PLAT_GIC_BASE + 0x2000)
#define PLAT_GICD_BASE			(PLAT_GIC_BASE + 0x1000)
#define PLAT_GICR_BASE			0

#define PLAT_SYS_COUNTER_FREQ_IN_TICKS	(400000000)
#define PLAT_HZ_CONVERT_TO_MHZ	(1000000)

/*******************************************************************************
 * SDMMC related pointer function
 ******************************************************************************/
#define SDMMC_READ_BLOCKS	mmc_read_blocks
#define SDMMC_WRITE_BLOCKS	mmc_write_blocks

/*******************************************************************************
 * sysmgr.boot_scratch_cold6 & 7 (64bit) are used to indicate L2 reset
 * is done and HPS should trigger warm reset via RMR_EL3.
 ******************************************************************************/
#define L2_RESET_DONE_REG			0xFFD12218

/* Platform specific system counter */
#define PLAT_SYS_COUNTER_FREQ_IN_MHZ	get_cpu_clk()

#endif /* PLATSOCFPGA_DEF_H */

