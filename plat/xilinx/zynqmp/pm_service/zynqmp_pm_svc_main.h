/*
 * Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ZYNQMP_PM_SVC_MAIN_H
#define ZYNQMP_PM_SVC_MAIN_H

#include "pm_common.h"

int32_t pm_setup(void);
uint64_t pm_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, const void *cookie, void *handle,
			uint64_t flags);
#endif /* ZYNQMP_PM_SVC_MAIN_H */
