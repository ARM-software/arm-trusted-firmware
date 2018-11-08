/*
 * Copyright (C)  2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MVEBU_H
#define MVEBU_H

/* Use this functions only when printf is allowed */
#define debug_enter()	VERBOSE("----> Enter %s\n", __func__)
#define debug_exit()	VERBOSE("<---- Exit  %s\n", __func__)

/* Macro for testing alignment. Positive if number is NOT aligned */
#define IS_NOT_ALIGN(number, align)	((number) & ((align) - 1))

/* Macro for alignment up. For example, ALIGN_UP(0x0330, 0x20) = 0x0340 */
#define ALIGN_UP(number, align) (((number) & ((align) - 1)) ? \
		(((number) + (align)) & ~((align)-1)) : (number))

/* Macro for testing whether a number is a power of 2. Positive if so */
#define IS_POWER_OF_2(number)	((number) != 0 && \
				(((number) & ((number) - 1)) == 0))

/*
 * Macro for ronding up to next power of 2
 * it is done by count leading 0 (clz assembly opcode) and see msb set bit.
 * then you can shift it left and get number which power of 2
 * Note: this Macro is for 32 bit number
 */
#define ROUND_UP_TO_POW_OF_2(number)	(1 << \
					(32 - __builtin_clz((number) - 1)))

#define _1MB_				(1024ULL * 1024ULL)
#define _1GB_				(_1MB_ * 1024ULL)
#define _2GB_				(2 * _1GB_)

#endif /* MVEBU_H */
