/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SOCFPGA_DEF_H
#define PLAT_SOCFPGA_DEF_H

#include <platform_def.h>

/* Platform Setting */
#define PLATFORM_MODEL		PLAT_SOCFPGA_STRATIX10

/* Register Mapping */
#define SOCFPGA_MMC_REG_BASE                    0xff808000

#define SOCFPGA_RSTMGR_OFST                     0xffd11000
#define SOCFPGA_RSTMGR_MPUMODRST_OFST           0xffd11020

#endif /* PLATSOCFPGA_DEF_H */

