/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <plat/common/platform.h>

/* The per_cpu_ptr_cache_t space allocation */
cpu_data_t percpu_data[PLATFORM_CORE_COUNT];

#ifndef __aarch64__
cpu_data_t *_cpu_data(void)
{
	return _cpu_data_by_index(plat_my_core_pos());
}
#endif
