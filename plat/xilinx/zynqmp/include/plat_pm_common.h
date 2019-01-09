/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
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

#define PAYLOAD_ARG_CNT		6U
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

#define ZYNQMP_TZ_VERSION_MAJOR		1
#define ZYNQMP_TZ_VERSION_MINOR		0
#define ZYNQMP_TZ_VERSION		((ZYNQMP_TZ_VERSION_MAJOR << 16) | \
					ZYNQMP_TZ_VERSION_MINOR)
#endif /* _PLAT_PM_COMMON_H_ */
