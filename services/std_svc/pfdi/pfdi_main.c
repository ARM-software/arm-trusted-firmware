/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>

#include <common/debug.h>
#include <services/pfdi.h>


void pfdi_init(void)
{
	assert(pfdi_func_desc.name != NULL);
	assert(pfdi_func_desc.run != NULL);
	assert(pfdi_func_desc.count != NULL);
	assert(pfdi_func_desc.result != NULL);

	NOTICE("PFDI: Initializing Platform Fault Detection Interface.\n");
}

pfdi_status_t pfdi_pe_test_part_count(uint64_t *tc_size)
{
	pfdi_status_t rc = 0;

	if (tc_size == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}
	rc = pfdi_func_desc.count(tc_size);
	if (rc == PFDI_RET_TEST_COUNT_ZERO) {
		*tc_size = 0U;
		return PFDI_SMCC_RET_SUCCESS;
	}

	return rc;
}

static pfdi_status_t pfdi_validate_test_run_args_internal(int64_t start,
	int64_t end,
	uint64_t mode,
	uint64_t *test_count,
	bool *full_range)
{
	pfdi_status_t rc;
	uint64_t tc;
	bool run_all;

	if (!is_valid_mode(mode)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	if ((start < -1) || (end < -1)) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	run_all = (start == -1) && (end == -1);
	if (((start == -1) || (end == -1)) && !run_all) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	rc = pfdi_pe_test_part_count(&tc);
	if (rc != 0) {
		return rc;
	}

	if (!run_all) {
		if (start > end) {
			return PFDI_RET_INVALID_PARAMETERS;
		}

		if (((uint64_t)start >= tc) ||
			((uint64_t)end >= tc)) {
			return PFDI_RET_INVALID_PARAMETERS;
		}
	}

	if (test_count != NULL) {
		*test_count = tc;
	}

	if (full_range != NULL) {
		*full_range = run_all;
	}

	return PFDI_SMCC_RET_SUCCESS;
}

pfdi_status_t pfdi_pe_test_run_validate(int64_t start, int64_t end, uint64_t mode)
{
	return pfdi_validate_test_run_args_internal(start, end, mode, NULL, NULL);
}

pfdi_status_t pfdi_pe_test_run(uint64_t start, uint64_t end, uint64_t mode,
				uint64_t *ft_id)
{
	pfdi_status_t rc = 0;
	uint64_t test_count;
	bool full_range;
	int64_t start_id = (int64_t)start;
	int64_t end_id = (int64_t)end;

	if (ft_id == NULL) {
		ERROR("PFDI: Invalid parameters: start=%" PRIu64
			", end=%" PRIu64 ", mode=%" PRIu64 "\n",
			start, end, mode);
		return PFDI_RET_INVALID_PARAMETERS;
	}

	rc = pfdi_validate_test_run_args_internal(start_id, end_id, mode,
		&test_count, &full_range);
	if (rc != 0) {
		return rc;
	}
	if (full_range) {
		if (test_count == 0U) {
			return 0;
		}

		start = 0U;
		end = test_count - 1U;
	}

	return pfdi_func_desc.run(start, end, mode, ft_id);
}

pfdi_status_t pfdi_pe_test_id(uint64_t *lib_version)
{
	if (lib_version == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	/*
	 * A zero/invalid test library version means this platform does not have
	 * usable metadata to report for PFDI_PE_TEST_ID.
	 */
	if (((PFDI_TEST_LIB_VERSION & PFDI_VERSION_ALLOWED_MASK) == UINT64_C(0)) ||
		((PFDI_TEST_LIB_VERSION & PFDI_UPPER_RESERVED_MASK) != UINT64_C(0)) ||
		((PFDI_TEST_LIB_VERSION & PFDI_RESERVED_MASK) != UINT64_C(0)) ||
		(PFDI_CFG_VENDOR != PFDI_VENDOR_ARM)) {
		*lib_version = 0ULL;
		return PFDI_RET_UNKNOWN;
	}

	*lib_version = 0ULL;

	pack_vendor_id(lib_version);

	if (*lib_version == 0ULL) {
		return PFDI_RET_UNKNOWN;
	}

	return (pfdi_status_t)0;
}

pfdi_status_t pfdi_pe_test_result(uint64_t *ft_id)
{
	uint64_t cpu_num;

	if (ft_id == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	*ft_id = UINT64_MAX;
	cpu_num = plat_my_core_pos();

	return pfdi_func_desc.result(cpu_num, ft_id);
}

pfdi_status_t pfdi_version(uint64_t *pfdi_version)
{
	if (pfdi_version == NULL) {
		return PFDI_RET_INVALID_PARAMETERS;
	}

	*pfdi_version = PFDI_VENDOR_VERSION;

	return (pfdi_status_t)0;
}

pfdi_status_t pfdi_pe_fw_check(void)
{
	/* No firmware checks implemented yet */
	return (pfdi_status_t)0;
}
