/*
 * Copyright (c) 2022, Arm Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * `delegated_attest_ns_interface_testsuite.c` and
 * `measured_boot_ns_interface_testsuite.c` are not added to the build directly.
 * but are included in this file, and this file is added to the build. This is
 * necessary because both files define the function `extra_tests_init`, so a
 * linker error occurs when both are linked to BL31. This file defines a macro
 * that renames the colliding function names to something unique.
 * `plat/arm/board/tc/rss_ap_tests.c` can call the test init functions with
 * their new name.
 */

#define register_testsuite_extra_ns_interface \
	register_testsuite_delegated_attest
#include <delegated_attest_ns_interface_testsuite.c>

#undef register_testsuite_extra_ns_interface
#define register_testsuite_extra_ns_interface \
	register_testsuite_measured_boot
#include <measured_boot_ns_interface_testsuite.c>