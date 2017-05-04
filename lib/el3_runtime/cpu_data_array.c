/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cassert.h>
#include <cpu_data.h>
#include <platform_def.h>

/* The per_cpu_ptr_cache_t space allocation */
cpu_data_t percpu_data[PLATFORM_CORE_COUNT];
