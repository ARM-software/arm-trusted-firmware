/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/cpus/cpu_ops.h>
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

/*
 * Initializes the cpu_ops_ptr if not already initialized in cpu_data.
 * May only be called after the data cache is enabled.
 */
void cpu_data_init_cpu_ops(void)
{
	struct cpu_ops *ops;

	if (get_cpu_data(cpu_ops_ptr) == NULL) {
		ops = get_cpu_ops_ptr();

		set_cpu_data(cpu_ops_ptr, ops);
	}
}
