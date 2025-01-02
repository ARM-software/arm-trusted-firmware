/*
 * Copyright (c) 2026, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <plat/common/platform.h>
#include <services/pfdi.h>
#include <services/pfdi_svc.h>
#include <services/std_svc.h>
#include <smccc_helpers.h>

uint64_t pfdi_smc_handler(uint32_t smc_fid,
			  u_register_t x1,
			  u_register_t x2,
			  u_register_t x3,
			  u_register_t x4,
			  void *cookie,
			  void *handle,
			  u_register_t flags)
{
	/* Set failed test id = invalid before invoking PFDI Function */
	uint64_t ft_id = UINT64_MAX;
	pfdi_status_t ret = 0;
	uint64_t x1_ret = 0;

	switch (smc_fid) {
	case PFDI_VERSION:
		uint64_t version = 0;

		if ((x1 != 0ULL) || (x2 != 0ULL) || (x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_version(&version);
		if (ret == PFDI_SMCC_RET_SUCCESS) {
			SMC_RET5(handle, version, 0U, 0U, 0U, 0U);
		}

		SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		break;
	case PFDI_FEATURES:
		if ((x2 != 0ULL) || (x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		SMC_RET5(handle, PFDI_SMCC_RET_NOT_SUPPORTED, 0U, 0U, 0U, 0U);
		break;
	case PFDI_PE_TEST_ID:
		uint64_t lib_version = 0;

		if ((x1 != 0ULL) || (x2 != 0ULL) || (x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_pe_test_id(&lib_version);
		if (ret != PFDI_SMCC_RET_SUCCESS) {
			SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		}

		SMC_RET5(handle, PFDI_SMCC_RET_SUCCESS, lib_version, 0U, 0U, 0U);
		break;
	case PFDI_PE_TEST_PART_COUNT:
		uint64_t count = 0;

		if ((x1 != 0ULL) || (x2 != 0ULL) || (x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_pe_test_part_count(&count);
		if (ret == PFDI_SMCC_RET_SUCCESS) {
			SMC_RET5(handle, count, 0U, 0U, 0U, 0U);
		}

		SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		break;
	case PFDI_PE_TEST_RUN:
		if ((x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_pe_test_run_validate((int64_t)x1, (int64_t)x2, PFDI_ONL_MODE);
		if (ret != PFDI_SMCC_RET_SUCCESS) {
			SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_pe_test_run(x1, x2, PFDI_ONL_MODE, &ft_id);
		if (ret == PFDI_RET_FAULT_FOUND) {
		/* Test has failed, but check if ft_id is updated */
			x1_ret = (ft_id == UINT64_MAX) ? PFDI_RET_UNKNOWN : ft_id;
			SMC_RET5(handle, PFDI_RET_FAULT_FOUND, x1_ret, 0U, 0U, 0U);
		}

		SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		break;
	case PFDI_PE_TEST_RESULT:
		if ((x1 != 0ULL) || (x2 != 0ULL) || (x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_pe_test_result(&ft_id);
		if (ret == PFDI_RET_FAULT_FOUND) {
			/* Test has failed, but check if ft_id is updated */
			x1_ret = (ft_id == UINT64_MAX) ? PFDI_RET_UNKNOWN : ft_id;
			SMC_RET5(handle, PFDI_RET_FAULT_FOUND, x1_ret, 0U, 0U, 0U);
		}

		SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		break;
	case PFDI_FW_CHECK:
		if ((x1 != 0ULL) || (x2 != 0ULL) || (x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}
		ret = pfdi_pe_fw_check();
		SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		break;
	case PFDI_FORCE_ERROR:
		if ((x3 != 0ULL) || (x4 != 0ULL)) {
			SMC_RET5(handle, PFDI_RET_INVALID_PARAMETERS, 0U, 0U, 0U, 0U);
		}

		ret = pfdi_pe_force_error_validate((uint32_t)x1, (int64_t)x2);
		if (ret != PFDI_SMCC_RET_SUCCESS) {
			SMC_RET5(handle, ret, 0U, 0U, 0U, 0U);
		}

		SMC_RET5(handle, PFDI_SMCC_RET_NOT_SUPPORTED, 0U, 0U, 0U, 0U);
		break;
	default:
		WARN("Unsupported PFDI Service Call: 0x%x\n", smc_fid);
		SMC_RET5(handle, PFDI_SMCC_RET_NOT_SUPPORTED, 0U, 0U, 0U, 0U);
		break;
	}
}
