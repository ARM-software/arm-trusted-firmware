/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */
#ifndef DRTM_MAIN_H
#define DRTM_MAIN_H

#include <stdint.h>

#include <lib/smccc.h>

enum drtm_retc {
	SUCCESS = SMC_OK,
	NOT_SUPPORTED = SMC_UNK,
	INVALID_PARAMETERS = -2,
	DENIED = -3,
	NOT_FOUND = -4,
	INTERNAL_ERROR = -5,
	MEM_PROTECT_INVALID = -6,
};

#endif /* DRTM_MAIN_H */
