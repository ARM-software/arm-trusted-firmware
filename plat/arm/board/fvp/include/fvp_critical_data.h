/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/nv_cntr_ids.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#define EVLOG_CRITICAL_DATA_STRING	"CRITICAL DATA"

#define CRITICAL_DATA_ID		CRITICAL_DATA_ID_BASE

struct fvp_critical_data {

	/* platform NV counters */
	unsigned int nv_ctr[MAX_NV_CTR_IDS];
};
