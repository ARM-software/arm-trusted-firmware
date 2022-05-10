/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SOCFPGA_DEF_H
#define PLAT_SOCFPGA_DEF_H

#include <platform_def.h>

/* Platform Setting */
#define PLATFORM_MODEL				PLAT_SOCFPGA_N5X
#define BOOT_SOURCE				BOOT_SOURCE_SDMMC

/* FPGA config helpers */
#define INTEL_SIP_SMC_FPGA_CONFIG_ADDR		0x400000
#define INTEL_SIP_SMC_FPGA_CONFIG_SIZE		0x2000000

/* Register Mapping */
#define SOCFPGA_CCU_NOC_REG_BASE		U(0xf7000000)
#define SOCFPGA_F2SDRAMMGR_REG_BASE		U(0xf8024000)

#define SOCFPGA_MMC_REG_BASE			U(0xff808000)

#define SOCFPGA_RSTMGR_REG_BASE			U(0xffd11000)
#define SOCFPGA_SYSMGR_REG_BASE			U(0xffd12000)

#define SOCFPGA_L4_PER_SCR_REG_BASE			U(0xffd21000)
#define SOCFPGA_L4_SYS_SCR_REG_BASE			U(0xffd21100)
#define SOCFPGA_SOC2FPGA_SCR_REG_BASE			U(0xffd21200)
#define SOCFPGA_LWSOC2FPGA_SCR_REG_BASE			U(0xffd21300)

/* Platform specific system counter */
/*
 * In N5X the clk init is done in Uboot SPL.
 * BL31 shall bypass the clk init and only provides other APIs.
 */
#define PLAT_SYS_COUNTER_FREQ_IN_MHZ	(400)

#endif /* PLAT_SOCFPGA_DEF_H */
