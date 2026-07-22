/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Build-Time Assertions
 *
 * This header provides a macro for performing compile-time assertions and
 * validation of constant expressions during the build process.
 */

#ifndef TI_BUILD_ASSERT_H
#define TI_BUILD_ASSERT_H

/*
 * Compile-time assertion that evaluates to zero
 *
 * Similar to CASSERT (include/lib/cassert.h) but differs in that it evaluates
 * to zero, making it usable inline within expressions. CASSERT generates a
 * typedef statement and cannot be used inside an expression.
 *
 * If the condition is false, the array has negative size causing a compilation
 * error. If true, evaluates to zero (sizeof(char[1]) - 1). Useful in macro
 * expressions where a build-time check is needed without affecting the value.
 */
#define TI_BUILD_ASSERT_OR_ZERO(cond) \
	(sizeof(char[(cond) ? 1 : -1]) - 1)

#endif /* TI_BUILD_ASSERT_H */
