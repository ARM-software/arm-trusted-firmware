/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM support for DRTM error remediation.
 *
 */
#include <inttypes.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include "drtm_main.h"
#include <plat/common/platform.h>

uint64_t drtm_set_error(uint64_t x1, void *ctx)
{
	int rc;

	rc = plat_set_drtm_error(x1);

	if (rc != 0) {
		SMC_RET1(ctx, INTERNAL_ERROR);
	}

	SMC_RET1(ctx, SUCCESS);
}

uint64_t drtm_get_error(void *ctx)
{
	uint64_t error_code;
	int rc;

	rc = plat_get_drtm_error(&error_code);

	if (rc != 0) {
		SMC_RET1(ctx, INTERNAL_ERROR);
	}

	SMC_RET2(ctx, SUCCESS, error_code);
}

void drtm_enter_remediation(uint64_t err_code, const char *err_str)
{
	int rc = plat_set_drtm_error(err_code);

	if (rc != 0) {
		ERROR("%s(): drtm_error_set() failed unexpectedly rc=%d\n",
		      __func__, rc);
		panic();
	}

	ERROR("DRTM: entering remediation of error:\n%" PRIu64 "\t\'%s\'\n",
	       err_code, err_str);

	ERROR("%s(): system reset is not yet supported\n", __func__);
	plat_system_reset();
}
