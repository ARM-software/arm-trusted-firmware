/*
 * Copyright (c) 2022, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdio.h>

#include <mbedtls_common.h>
#include <plat/common/platform.h>
#include <psa/crypto.h>
#include <rss_comms.h>

#include "rss_ap_testsuites.h"

static struct test_suite_t test_suites[] = {
	{.freg = register_testsuite_delegated_attest},
	{.freg = register_testsuite_measured_boot},
};

static void run_tests(void)
{
	enum test_suite_err_t ret;
	psa_status_t status;
	size_t i;

	rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE, PLAT_RSS_AP_RCV_MHU_BASE);
	mbedtls_init();
	status = psa_crypto_init();
	if (status != PSA_SUCCESS) {
		printf("\n\npsa_crypto_init failed (status = %d)\n", status);
		assert(false);
		plat_error_handler(-1);
	}

	for (i = 0; i < ARRAY_SIZE(test_suites); ++i) {
		struct test_suite_t *suite = &(test_suites[i]);

		suite->freg(suite);
		ret = run_testsuite(suite);
		if (ret != TEST_SUITE_ERR_NO_ERROR) {
			printf("\n\nError during executing testsuite '%s'.\n", suite->name);
			assert(false);
			plat_error_handler(-1);
		}
	}
	printf("\nAll tests are run.\n");
}

void run_platform_tests(void)
{
	size_t i;

	run_tests();

	printf("\n\n");

	/* Print a summary of all the tests that had been run. */
	printf("SUMMARY:\n");
	for (i = 0; i < ARRAY_SIZE(test_suites); ++i) {

		struct test_suite_t *suite = &(test_suites[i]);

		switch (suite->val) {
		case TEST_PASSED:
			printf("    %s PASSED.\n", suite->name);
			break;
		case TEST_FAILED:
			printf("    %s FAILED.\n", suite->name);
			break;
		case TEST_SKIPPED:
			printf("    %s SKIPPED.\n", suite->name);
			break;
		default:
			assert(false);
			break;
		}
	}

	printf("\n\n");
}
