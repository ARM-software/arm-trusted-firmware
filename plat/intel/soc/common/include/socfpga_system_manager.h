/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_SYSTEMMANAGER_H
#define SOCFPGA_SYSTEMMANAGER_H

#include "socfpga_plat_def.h"

/* System Manager Register Map */

#define SOCFPGA_SYSMGR_SDMMC				0x28

/* Field Masking */
#define SYSMGR_SDMMC_DRVSEL(x)			(((x) & 0x7) << 0)
#define SYSMGR_SDMMC_SMPLSEL(x)			(((x) & 0x7) << 4)

#define IDLE_DATA_LWSOC2FPGA				BIT(4)
#define IDLE_DATA_SOC2FPGA				BIT(0)
#define IDLE_DATA_MASK		(IDLE_DATA_LWSOC2FPGA | IDLE_DATA_SOC2FPGA)

#define SYSMGR_QSPI_REFCLK_MASK				GENMASK(27, 0)

#define SYSMGR_ECC_OCRAM_MASK				BIT(1)
#define SYSMGR_ECC_DDR0_MASK				BIT(16)
#define SYSMGR_ECC_DDR1_MASK				BIT(17)

/* Macros */

#define SOCFPGA_SYSMGR(_reg)		(SOCFPGA_SYSMGR_REG_BASE \
						+ (SOCFPGA_SYSMGR_##_reg))

#endif /* SOCFPGA_SYSTEMMANAGER_H */
