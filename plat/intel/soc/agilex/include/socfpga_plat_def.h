/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SOCFPGA_DEF_H
#define PLAT_SOCFPGA_DEF_H

#include <platform_def.h>

/* Platform Setting */
#define PLATFORM_MODEL				PLAT_SOCFPGA_AGILEX
#define BOOT_SOURCE				BOOT_SOURCE_SDMMC

/* Register Mapping */
#define SOCFPGA_MMC_REG_BASE			0xff808000

#define SOCFPGA_RSTMGR_REG_BASE			0xffd11000
#define SOCFPGA_SYSMGR_REG_BASE			0xffd12000

#define SOCFPGA_L4_PER_SCR_REG_BASE             0xffd21000
#define SOCFPGA_L4_SYS_SCR_REG_BASE             0xffd21100
#define SOCFPGA_SOC2FPGA_SCR_REG_BASE           0xffd21200
#define SOCFPGA_LWSOC2FPGA_SCR_REG_BASE         0xffd21300

#endif /* PLAT_SOCFPGA_DEF_H */

