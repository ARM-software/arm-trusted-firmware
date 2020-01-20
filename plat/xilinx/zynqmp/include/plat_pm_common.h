/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
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

#if ZYNQMP_IPI_CRC_CHECK
#define PAYLOAD_ARG_CNT         8U
#define IPI_W0_TO_W6_SIZE       28U
#define PAYLOAD_CRC_POS         7U
#define CRC_INIT_VALUE          0x4F4EU
#define CRC_ORDER               16U
#define CRC_POLYNOM             0x8005U
#else
#define PAYLOAD_ARG_CNT         6U
#endif
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

#define ZYNQMP_TZ_VERSION_MAJOR		1
#define ZYNQMP_TZ_VERSION_MINOR		0
#define ZYNQMP_TZ_VERSION		((ZYNQMP_TZ_VERSION_MAJOR << 16) | \
					ZYNQMP_TZ_VERSION_MINOR)
#endif /* _PLAT_PM_COMMON_H_ */
