/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/per_cpu/per_cpu.h>

__pure void *per_cpu_by_index_compute(uint32_t cpu, const void *addr)
{
	const char *cpu_base;
#ifndef __aarch64__
	cpu_base = ((const char *)PER_CPU_START +
				((cpu) * (PER_CPU_UNIT_SIZE)));

#else
	cpu_base = per_cpu_base(cpu);
#endif
	return (void *)(cpu_base + PER_CPU_OFFSET(addr));
}

__pure void *per_cpu_cur_compute(const void *addr)
{
#ifndef __aarch64__
	return (per_cpu_by_index_compute(plat_my_core_pos(), addr));
#else
	return (void *)((read_tpidr_el3() + PER_CPU_OFFSET(addr)));
#endif
}

