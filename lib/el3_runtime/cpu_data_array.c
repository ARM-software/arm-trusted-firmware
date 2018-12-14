/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>

/* The per_cpu_ptr_cache_t space allocation */
cpu_data_t percpu_data[PLATFORM_CORE_COUNT];
