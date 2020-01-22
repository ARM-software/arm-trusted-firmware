/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains platform specific definitions of commonly used macros data types
 * for PU Power Management. This file should be common for all PU's.
 */

#ifndef PLAT_PM_COMMON_H
#define PLAT_PM_COMMON_H

#include <common/debug.h>
#include <stdint.h>
#include "pm_defs.h"

#define PAYLOAD_ARG_CNT		6U
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

#define VERSAL_TZ_VERSION_MAJOR		1
#define VERSAL_TZ_VERSION_MINOR		0
#define VERSAL_TZ_VERSION		((VERSAL_TZ_VERSION_MAJOR << 16) | \
					VERSAL_TZ_VERSION_MINOR)
#endif /* PLAT_PM_COMMON_H */
