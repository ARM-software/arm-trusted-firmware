/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC_PLAT_H
#define TC_PLAT_H

void tc_bl31_common_platform_setup(void);

#ifdef PLATFORM_TEST
void run_platform_tests(void);
#endif

#endif /* TC_PLAT_H */
