/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2019, Arm Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDINT_H
#define STDINT_H

#include <limits.h>
#include <lib/utils_def.h>

#define INT64_MAX  LONG_MAX
#define INT64_MIN  LONG_MIN
#define UINT64_MAX ULONG_MAX

#define INT64_C(x) x ## L
#define UINT64_C(x) x ## UL

typedef long int64_t;
typedef unsigned long uint64_t;
typedef long int64_least_t;
typedef unsigned long uint64_least_t;
typedef long int64_fast_t;
typedef unsigned long uint64_fast_t;

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

#define INT8_MAX  CHAR_MAX
#define INT8_MIN  CHAR_MIN
#define UINT8_MAX UCHAR_MAX

#define INT16_MAX  SHRT_MAX
#define INT16_MIN  SHRT_MIN
#define UINT16_MAX USHRT_MAX

#define INT32_MAX  INT_MAX
#define INT32_MIN  INT_MIN
#define UINT32_MAX UINT_MAX

#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST8_MAX  INT8_MAX
#define UINT_LEAST8_MAX UINT8_MAX

#define INT_LEAST16_MIN  INT16_MIN
#define INT_LEAST16_MAX  INT16_MAX
#define UINT_LEAST16_MAX UINT16_MAX

#define INT_LEAST32_MIN  INT32_MIN
#define INT_LEAST32_MAX  INT32_MAX
#define UINT_LEAST32_MAX UINT32_MAX

#define INT_LEAST64_MIN  INT64_MIN
#define INT_LEAST64_MAX  INT64_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN  INT32_MIN
#define INT_FAST8_MAX  INT32_MAX
#define UINT_FAST8_MAX UINT32_MAX

#define INT_FAST16_MIN  INT32_MIN
#define INT_FAST16_MAX  INT32_MAX
#define UINT_FAST16_MAX UINT32_MAX

#define INT_FAST32_MIN  INT32_MIN
#define INT_FAST32_MAX  INT32_MAX
#define UINT_FAST32_MAX UINT32_MAX

#define INT_FAST64_MIN  INT64_MIN
#define INT_FAST64_MAX  INT64_MAX
#define UINT_FAST64_MAX UINT64_MAX

#define INTPTR_MIN  LONG_MIN
#define INTPTR_MAX  LONG_MAX
#define UINTPTR_MAX ULONG_MAX

#define INTMAX_MIN  LLONG_MIN
#define INTMAX_MAX  LLONG_MAX
#define UINTMAX_MAX ULLONG_MAX

#define PTRDIFF_MIN LONG_MIN
#define PTRDIFF_MAX LONG_MAX

#define SIZE_MAX ULONG_MAX

#define INT8_C(x)  x
#define INT16_C(x) x
#define INT32_C(x) x

#define UINT8_C(x)  x
#define UINT16_C(x) x
#define UINT32_C(x) x ## U

#define INTMAX_C(x)  x ## LL
#define UINTMAX_C(x) x ## ULL

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef signed char int8_least_t;
typedef short int16_least_t;
typedef int int32_least_t;

typedef unsigned char uint8_least_t;
typedef unsigned short uint16_least_t;
typedef unsigned int uint32_least_t;

typedef int int8_fast_t;
typedef int int16_fast_t;
typedef int int32_fast_t;

typedef unsigned int uint8_fast_t;
typedef unsigned int uint16_fast_t;
typedef unsigned int uint32_fast_t;

typedef int int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;
typedef long int_fast64_t;

typedef int int_least8_t;
typedef int int_least16_t;
typedef int int_least32_t;
typedef long int_least64_t;

typedef unsigned int uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef unsigned long uint_fast64_t;

typedef unsigned int uint_least8_t;
typedef unsigned int uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long uint_least64_t;

typedef long intptr_t;
typedef unsigned long uintptr_t;

/*
* Conceptually, these are supposed to be the largest integers representable in C,
* but GCC and Clang define them as long long for compatibility.
*/
typedef long long intmax_t;
typedef unsigned long long uintmax_t;

typedef long register_t;
typedef unsigned long u_register_t;


#define SZ_32				U(0x00000020)
#define SZ_64				U(0x00000040)
#define SZ_128				U(0x00000080)
#define SZ_256				U(0x00000100)
#define SZ_512				U(0x00000200)

#define SZ_1K				U(0x00000400)
#define SZ_2K				U(0x00000800)
#define SZ_4K				U(0x00001000)
#define SZ_8K				U(0x00002000)
#define SZ_16K				U(0x00004000)
#define SZ_32K				U(0x00008000)
#define SZ_64K				U(0x00010000)
#define SZ_128K				U(0x00020000)
#define SZ_256K				U(0x00040000)
#define SZ_512K				U(0x00080000)

#define SZ_1M				U(0x00100000)
#define SZ_2M				U(0x00200000)
#define SZ_4M				U(0x00400000)
#define SZ_8M				U(0x00800000)
#define SZ_16M				U(0x01000000)
#define SZ_32M				U(0x02000000)
#define SZ_64M				U(0x04000000)
#define SZ_128M				U(0x08000000)
#define SZ_256M				U(0x10000000)
#define SZ_512M				U(0x20000000)

#define SZ_1G				U(0x40000000)
#define SZ_2G				U(0x80000000)

#endif /* STDINT_H */