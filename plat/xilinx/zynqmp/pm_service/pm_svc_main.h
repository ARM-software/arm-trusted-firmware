/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PM_SVC_MAIN_H_
#define _PM_SVC_MAIN_H_

#include "pm_common.h"

int pm_setup(void);
uint64_t pm_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, void *cookie, void *handle,
			uint64_t flags);

#endif /*  _PM_SVC_MAIN_H_ */
