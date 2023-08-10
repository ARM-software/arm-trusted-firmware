/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PM_SVC_MAIN_H
#define PM_SVC_MAIN_H

#include <pm_common.h>

/******************************************************************************/
/**
 * SECURE_REDUNDANT_CALL() - Adds redundancy to the function call. This is to
 *			     avoid glitches which can skip a function call
 *			     and cause altering of the code flow in security
 *			     critical functions.
 * @status: Variable which holds the return value of function executed
 * @status_tmp: Variable which holds the return value of redundant function
 *		call executed
 * @function: Function to be executed
 *
 * Return: None
 *
 ******************************************************************************/
#define SECURE_REDUNDANT_CALL(status, status_tmp, function, ...)   \
	{ \
		status = function(__VA_ARGS__); \
		status_tmp = function(__VA_ARGS__); \
	}

int32_t pm_setup(void);
uint64_t pm_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
			uint64_t x4, const void *cookie, void *handle,
			uint64_t flags);

int32_t pm_register_sgi(uint32_t sgi_num, uint32_t reset);
#endif /* PM_SVC_MAIN_H */
