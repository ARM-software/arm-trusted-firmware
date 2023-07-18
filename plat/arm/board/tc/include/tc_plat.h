/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC_PLAT_H
#define TC_PLAT_H

#ifdef PLATFORM_TEST_ROTPK
#include <rss_crypto_defs.h>
#endif

void tc_bl31_common_platform_setup(void);

#ifdef PLATFORM_TEST_TFM_TESTSUITE
int run_platform_tests(void);
#endif

#ifdef PLATFORM_TEST_NV_COUNTERS
int nv_counter_test(void);
#endif

#ifdef PLATFORM_TEST_ROTPK
struct key_id_info {
	enum rss_key_id_builtin_t key_id;
	const char *key_id_name;
};

int rotpk_test(void);
#endif

#endif /* TC_PLAT_H */
