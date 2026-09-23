/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This file provides a placeholder (dummy) implementation of the PFDI
 * CPU self-test interfaces. It is intended for reference or simulation use
 * only, and does not perform actual fault detection.
 */

#include <inttypes.h>
#include <common/debug.h>
#include <services/pfdi.h>

#define LIB_NAME        "PFDI_TESTS"
#define MAX_NUM_TESTS   U(41)

typedef struct {
	pfdi_status_t status;
	uint64_t ft_id;
} pfdi_tests_results_t;

static pfdi_tests_results_t oor_pfdi_tests_results[PLATFORM_CORE_COUNT] = {
	[0 ... PLATFORM_CORE_COUNT - 1] = {
		.status = PFDI_RET_NOT_RUN,
		.ft_id  = UINT64_MAX
	}
};

static bool cpu_self_test_initialized[PLATFORM_CORE_COUNT];

/*
 * Keep the upstream Aspen platform buildable without a hardware self-test
 * library. Platforms can override PFDI_SOURCE to replace this file with a real
 * implementation.
 */
static pfdi_status_t cpu_self_test_init(uint32_t cpu_num)
{
	(void)cpu_num;
	return 0;
}

static pfdi_status_t cpu_self_test_run(uint32_t cpu_num, uint32_t mode,
				       uint64_t start, uint64_t end,
				       uint64_t *ft_id)
{
	(void)cpu_num;
	(void)mode;
	(void)start;
	(void)end;

	if (ft_id != NULL) {
		*ft_id = UINT64_MAX;
	}

	return 0;
}

static pfdi_status_t pfdi_cpu_self_test_run(uint64_t start, uint64_t end,
					    uint64_t mode, uint64_t *ft_id)
{
	uint64_t cpu_num = plat_my_core_pos();
	pfdi_status_t status;

	if (ft_id != NULL) {
		*ft_id = UINT64_MAX;
	}

	if (cpu_num >= PLATFORM_CORE_COUNT) {
		WARN("%s: Invalid CPU index: %" PRIu64 "\n", LIB_NAME, cpu_num);
		return PFDI_RET_ERROR;
	}

	if (!cpu_self_test_initialized[cpu_num]) {
		status = cpu_self_test_init((uint32_t)cpu_num);
		if (status == 0 && mode == PFDI_ONL_MODE) {
			cpu_self_test_initialized[cpu_num] = true;
		} else if (status != 0) {
			goto exit;
		}
	}

	status = cpu_self_test_run((uint32_t)cpu_num, (uint32_t)mode,
				   start, end, ft_id);

exit:
	if (mode == PFDI_OOR_MODE) {
		oor_pfdi_tests_results[cpu_num].status = status;
		oor_pfdi_tests_results[cpu_num].ft_id =
			(ft_id != NULL) ? *ft_id : UINT64_MAX;
	}

	return status;
}

static pfdi_status_t pfdi_cpu_self_test_count(uint64_t *testcase_count)
{
	if (testcase_count == NULL) {
		return PFDI_RET_ERROR;
	}

	if (MAX_NUM_TESTS == 0U) {
		return PFDI_RET_TEST_COUNT_ZERO;
	}

	*testcase_count = (uint64_t)MAX_NUM_TESTS;

	return 0;
}

static pfdi_status_t pfdi_cpu_self_test_result(uint64_t cpu_num, uint64_t *ft_id)
{
	if (cpu_num >= PLATFORM_CORE_COUNT) {
		WARN("%s: Invalid CPU index: %" PRIu64 "\n", LIB_NAME, cpu_num);
		return PFDI_RET_ERROR;
	}

	if (ft_id != NULL) {
		*ft_id = oor_pfdi_tests_results[cpu_num].ft_id;
	}

	return oor_pfdi_tests_results[cpu_num].status;
}

REGISTER_PFDI_FUNC(LIB_NAME,
		   pfdi_cpu_self_test_run,
		   pfdi_cpu_self_test_count,
		   pfdi_cpu_self_test_result);
