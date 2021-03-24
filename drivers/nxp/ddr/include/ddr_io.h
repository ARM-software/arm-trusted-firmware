/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_IO_H
#define DDR_IO_H

#include <endian.h>

#include <lib/mmio.h>

#define min(a, b)  (((a) > (b)) ? (b) : (a))

#define max(a, b)  (((a) > (b)) ? (a) : (b))

/* macro for memory barrier */
#define mb()		asm volatile("dsb sy" : : : "memory")

#ifdef NXP_DDR_BE
#define ddr_in32(a)			bswap32(mmio_read_32((uintptr_t)(a)))
#define ddr_out32(a, v)			mmio_write_32((uintptr_t)(a),\
							bswap32(v))
#elif defined(NXP_DDR_LE)
#define ddr_in32(a)			mmio_read_32((uintptr_t)(a))
#define ddr_out32(a, v)			mmio_write_32((uintptr_t)(a), v)
#else
#error Please define CCSR DDR register endianness
#endif

#define ddr_setbits32(a, v)		ddr_out32((a), ddr_in32(a) | (v))
#define ddr_clrbits32(a, v)		ddr_out32((a), ddr_in32(a) & ~(v))
#define ddr_clrsetbits32(a, c, s)	ddr_out32((a), (ddr_in32(a) & ~(c)) \
						  | (s))

#endif /*	DDR_IO_H	*/
