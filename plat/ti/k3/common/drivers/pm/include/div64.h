/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DIV64_H
#define DIV64_H

#include <types/short_types.h>
#include <compiler.h>


/**
 * 64-bit division/modulo.
 *
 * Perform a 64-bit division/modulo operation. This divides a 64-bit numerator
 * by a 32-bit denominator and provides the result and remainder. Note that
 * passing the value 0 for denominator will result in an exception.
 *
 * @param rem The numerator should be passed in this memory location. The
 * remainder is returned in this memory location.
 *
 * @param b The denominator.
 *
 * @return The result of the division.
 */
uint64_t pm_div64(uint64_t *rem, uint32_t b);

/**
 * 64-bit division.
 *
 * Perform a 64-bit division operation. This divides a 64-bit numerator
 * by a 32-bit denominator and provides the result. Note that passing
 * the value 0 for denominator will result in an exception.
 *
 * @param a The numerator.
 *
 * @param b The denominator.
 *
 * @return The result of the division.
 */
static inline uint64_t div64(uint64_t a, uint32_t b)
{
	return pm_div64(&a, b);
}

/**
 * 64-bit modulo.
 *
 * Perform a 64-bit division operation. This divides a 64-bit numerator
 * by a 32-bit denominator and provides the remainder. Note that passing
 * the value 0 for denominator will result in an exception.
 *
 * @param a The numerator.
 *
 * @param b The denominator.
 *
 * @return The remainder of the division.
 */
static inline uint64_t mod64(uint64_t a, uint32_t b)
{
	uint64_t temp UNUSED;

	temp = pm_div64(&a, b);
	return a;
}


#endif
