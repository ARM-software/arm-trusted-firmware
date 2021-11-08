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

#define INT64_MAX  LLONG_MAX
#define INT64_MIN  LLONG_MIN
#define UINT64_MAX ULLONG_MAX

#define INT64_C(x) x ## LL
#define UINT64_C(x) x ## ULL

typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef long long int64_least_t;
typedef unsigned long long uint64_least_t;
typedef long long int64_fast_t;
typedef unsigned long long uint64_fast_t;

#endif
