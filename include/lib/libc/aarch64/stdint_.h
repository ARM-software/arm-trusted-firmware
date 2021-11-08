/*
 * Copyright 2020 Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2020, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDINT__H
#define STDINT__H

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

#endif
