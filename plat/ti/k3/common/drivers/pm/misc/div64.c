/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <div64.h>

uint64_t pm_div64(uint64_t *remainder64, uint32_t b32)
{
	uint32_t ah32, al32;
	uint64_t a64;
	uint64_t b64;
	uint64_t bit64;
	uint64_t result64;

	if (*remainder64 == 0ULL) {
		result64 = 0ULL;
	} else {
		if (b32 == 0U) {
			/* Generate exception */
			result64 = 1ULL / (uint64_t) b32;
		} else {
			a64 = *remainder64;

			ah32 = (uint32_t) (a64 >> 32U);
			al32 = (uint32_t) (a64 & 0xffffffffULL);

			result64 = 0ULL;

			/* Check if b * 2^32 fits at least once */
			if (ah32 >= b32) {
				/* Divide out b * 2^32 */
				ah32 /= b32;
				result64 = ((uint64_t) ah32) << 32U;
				a64 -= result64 * b32;
			}

			/* 32 bit division possible */
			if (ah32 == 0U) {
				a64 = (uint64_t) al32 % (uint64_t) b32;
				result64 += (uint64_t) al32 / (uint64_t) b32;
			} else {
				/* Time for long division */
				b64 = ((uint64_t) b32) << 32U;
				bit64 = 1ULL << 32;

				/* Shift divisor up to line up with remaining dividend */
				while (b64 < a64) {
					b64 <<= 1U;
					bit64 <<= 1U;
				}

				/* Long division loop */
				do {
					if (b64 <= a64) {
						a64 -= b64;
						result64 += bit64;
					}
					b64 >>= 1U;
					bit64 >>= 1U;
				} while (bit64 != 0ULL);
			}

			*remainder64 = a64;
		}
	}

	return result64;
}
