/*
 * Copyright (c) 2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains platform specific definitions of commonly used macros data types
 * for PU Power Management. This file should be common for all PU's.
 */

#ifndef PLAT_PM_COMMON_H
#define PLAT_PM_COMMON_H

#include <stdint.h>

#include <common/debug.h>

#include "pm_defs.h"

/* Processor core device IDs */
#define PM_DEV_CLUSTER0_ACPU_0	(0x1810C0AFU)
#define PM_DEV_CLUSTER0_ACPU_1	(0x1810C0B0U)

#define PM_DEV_CLUSTER1_ACPU_0	(0x1810C0B3U)
#define PM_DEV_CLUSTER1_ACPU_1	(0x1810C0B4U)

#define PM_DEV_CLUSTER2_ACPU_0	(0x1810C0B7U)
#define PM_DEV_CLUSTER2_ACPU_1	(0x1810C0B8U)

#define PM_DEV_CLUSTER3_ACPU_0	(0x1810C0BBU)
#define PM_DEV_CLUSTER3_ACPU_1	(0x1810C0BCU)

#endif /* PLAT_PM_COMMON_H */
