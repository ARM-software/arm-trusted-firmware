/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_SVC_MAIN_H
#define PM_SVC_MAIN_H

#include <pm_common.h>

int pm_setup(void);
uint64_t pm_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, void *cookie, void *handle,
			uint64_t flags);

int pm_register_sgi(unsigned int sgi_num);
#endif /* PM_SVC_MAIN_H */
