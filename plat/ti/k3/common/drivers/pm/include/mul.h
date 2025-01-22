/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MUL_H
#define MUL_H


/*
 * \brief Multiply and return overflow status.
 *
 * Multiply two 32-bit numbers and return true if the value did not fit in
 * the 32-bit result.
 *
 * Note that ARM does not provide a flag on multiplication overflow. We
 * instead use umull to produce a 64-bit result from two 32-bit operands.
 * umull just takes a couple extra cycles vs a 32-bit umul. We create
 * this as an inline to allow the compiler to optimize it properly.
 *
 * \param a
 * First operand.
 *
 * \param b
 * Second operand.
 *
 * \param c
 * Pointer to location to store result.
 *
 * \return
 * True if overflow occurred, false otherwise
 */
static inline bool mul32_check_overflow(uint32_t a, uint32_t b, uint32_t *ret)
{
	uint64_t d = (uint64_t) a * b;

	*ret = (uint32_t) d;
	return d > 0xffffffffULL;
}

#endif
