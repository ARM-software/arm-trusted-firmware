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

/* Register Mapping */
#define SOCFPGA_MMC_REG_BASE			0xff808000

#define SOCFPGA_RSTMGR_OFST			0xffd11000
#define SOCFPGA_RSTMGR_MPUMODRST_OFST		0xffd11020

#endif /* PLAT_SOCFPGA_DEF_H */

