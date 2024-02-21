
/*
 * Copyright (c) 2022, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RSS_AP_TESTSUITES_H
#define RSS_AP_TESTSUITES_H

#include <test_framework.h>

void register_testsuite_measured_boot(struct test_suite_t *p_test_suite);
void register_testsuite_delegated_attest(struct test_suite_t *p_test_suite);

#endif /* RSS_AP_TESTSUITES_H */
