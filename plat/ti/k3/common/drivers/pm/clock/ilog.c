/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "ilog.h"
#include <limits.h>
#include <types/short_types.h>

/*The fastest fallback strategy for platforms with fast multiplication appears
 * to be based on de Bruijn sequences~\cite{LP98}.
 * Tests confirmed this to be true even on an ARM11, where it is actually faster
 * than using the native clz instruction.
 * Define ILOG_NODEBRUIJN to use a simpler fallback on platforms where
 * multiplication or table lookups are too expensive.
 *
 * @UNPUBLISHED{LP98,
 *  author="Charles E. Leiserson and Harald Prokop",
 *  title="Using de {Bruijn} Sequences to Index a 1 in a Computer Word",
 *  month=Jun,
 *  year=1998,
 *  note="\url{http://supertech.csail.mit.edu/papers/debruijn.pdf}"
 * }*/
static const UNNEEDED uint8_t DEBRUIJN_IDX32[32] = {
	0,  1,	28, 2,	29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9
};

/* We always compile these in, in case someone takes address of function. */
#undef ilog32_nz
#undef ilog32
#undef ilog64_nz
#undef ilog64

int32_t ilog32(uint32_t _v)
{
	uint32_t _v_val_p = _v;

/*On a Pentium M, this branchless version tested as the fastest version without
 * multiplications on 1,000,000,000 random 32-bit integers, edging out a
 * similar version with branches, and a 256-entry LUT version.*/
# if defined(ILOG_NODEBRUIJN)
	uint32_t ret;
	uint32_t m;

	ret = _v_val_p > 0;
	m = (_v_val_p > 0xFFFFU) << 4;
	_v_val_p >>= m;
	ret |= m;
	m = (_v_val_p > 0xFFU) << 3;
	_v_val_p >>= m;
	ret |= m;
	m = (_v_val_p > 0xFU) << 2;
	_v_val_p >>= m;
	ret |= m;
	m = (_v_val_p > 3) << 1;
	_v_val_p >>= m;
	ret |= m;
	ret += _v_val_p > 1;
	return (int32_t) ret;
/*This de Bruijn sequence version is faster if you have a fast multiplier.*/
# else
	uint32_t ret;

	ret = ((_v_val_p > 0U) ? 1U : 0U);
	_v_val_p |= _v_val_p >> 1;
	_v_val_p |= _v_val_p >> 2;
	_v_val_p |= _v_val_p >> 4;
	_v_val_p |= _v_val_p >> 8;
	_v_val_p |= _v_val_p >> 16;
	_v_val_p = (_v_val_p >> 1) + 1U;
	ret += DEBRUIJN_IDX32[((_v_val_p * 0x77CB531U) >> 27) & 0x1FU];
	return (int32_t) ret;
# endif
}

int32_t ilog32_nz(uint32_t _v)
{
	return ilog32(_v);
}

int32_t ilog64(uint64_t _v)
{
# if defined(ILOG_NODEBRUIJN)
	uint32_t v;
	uint32_t ret;
	uint32_t m;

	ret = (uint32_t) (_v > 0);
	m = (uint32_t) (_v > 0xFFFFFFFFU) << 5;
	v = (uint32_t) (_v >> m);
	ret |= m;
	m = (v > 0xFFFFU) << 4;
	v >>= m;
	ret |= m;
	m = (v > 0xFFU) << 3;
	v >>= m;
	ret |= m;
	m = (v > 0xFU) << 2;
	v >>= m;
	ret |= m;
	m = (v > 3) << 1;
	v >>= m;
	ret |= m;
	ret += v > 1;
	return (int32_t) ret;
# else
	uint32_t v;
	uint32_t ret;
	uint32_t m;

	ret = ((_v > 0U) ? 1U : 0U);
	m = (uint32_t) ((_v > 0xFFFFFFFFU) ? 1U : 0U) << 5;
	v = ((uint32_t) _v >> m);
	ret |= m;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v = (v >> 1) + 1U;
	ret += DEBRUIJN_IDX32[((v * 0x77CB531U) >> 27) & 0x1FU];
	return (int32_t) ret;
# endif
}

int32_t ilog64_nz(uint64_t _v)
{
	return ilog64(_v);
}
