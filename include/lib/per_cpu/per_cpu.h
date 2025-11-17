/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_H
#define PER_CPU_H

#include <stddef.h>

#include <plat/common/platform.h>

#include <common/bl_common.h>
#include <lib/per_cpu/per_cpu_defs.h>

/* section where per-cpu objects using per-cpu framework would reside */
#define PER_CPU_SECTION_NAME	".per_cpu"

/* per cpu section size for a single CPU/PE */
#define PER_CPU_UNIT_SIZE ((size_t)(PER_CPU_UNIT_END - PER_CPU_START))
#define PER_CPU_OFFSET(x) ((ptrdiff_t)((uintptr_t)(x) - PER_CPU_START))

/*******************************************************************************
 * per-cpu definer and accessor interfaces
 ******************************************************************************/

/* Declare a per-cpu object */
#define PER_CPU_DECLARE(TYPE, NAME)				\
	extern typeof(TYPE) NAME

/* Define a per-cpu object */
#define PER_CPU_DEFINE(TYPE, NAME)				\
	typeof(TYPE) NAME					\
	__section(PER_CPU_SECTION_NAME)

/* Get a pointer to a per-cpu object for a given CPU */
#define PER_CPU_BY_INDEX(NAME, CPU)				\
	((__typeof__(&(NAME)))					\
	(per_cpu_by_index_compute((CPU), (const void *)&(NAME))))

/* Get a pointer to a per-cpu object for the current CPU */
#define PER_CPU_CUR(NAME)					\
	((__typeof__(&(NAME)))					\
	(per_cpu_cur_compute((const void *)&(NAME))))

/*******************************************************************************
 * Functions
 ******************************************************************************/

__pure const char *per_cpu_base(uint32_t cpu);
__pure void *per_cpu_by_index_compute(uint32_t cpu, const void *addr);
__pure void *per_cpu_cur_compute(const void *addr);

#endif /* PER_CPU_H */
