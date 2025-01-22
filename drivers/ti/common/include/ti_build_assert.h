/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Build-Time Assertions
 *
 * This header provides a macro for performing compile-time assertions and
 * validation of constant expressions during the build process.
 */

#ifndef BUILD_ASSERT_H
#define BUILD_ASSERT_H

/**
 * \brief Compile-time assertion that evaluates to zero
 *
 * Performs a compile-time assertion by creating an array with size based on
 * the condition. If the condition is false, the array has negative size,
 * causing a compilation error. If true, evaluates to zero (sizeof(char[1]) - 1).
 * Useful in macro expressions where a build-time check is needed without
 * affecting the computed value.
 *
 * \param cond Constant expression to assert at compile time
 *
 * \return 0 if condition is true (compilation fails if false)
 */
#define BUILD_ASSERT_OR_ZERO(cond) \
	(sizeof(char[(cond) ? 1 : -1]) - 1)

#endif /* BUILD_ASSERT_H */
