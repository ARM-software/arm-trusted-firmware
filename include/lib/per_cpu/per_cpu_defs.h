/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PER_CPU_DEFS_H
#define PER_CPU_DEFS_H

#include <platform_def.h>

/*
 * The NUMA aware per-cpu framework uses the term NODE to denote a group of CPUs
 * that have uniform proximity to a given memory node. Without NUMA awareness,
 * all CPUs on the system fall within the same NODE. With NUMA awareness, it is
 * the platforms responsibility to define how CPUs are grouped on a per NODE
 * basis.
 */
#define PER_CPU_NODE_CORE_COUNT    (PLATFORM_CORE_COUNT / PLATFORM_NODE_COUNT)

#endif
