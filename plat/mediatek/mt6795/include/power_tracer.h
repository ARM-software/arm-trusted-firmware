/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef POWER_TRACER_H
#define POWER_TRACER_H

#define CPU_UP		0
#define CPU_DOWN	1
#define CPU_SUSPEND	2
#define CLUSTER_UP	3
#define CLUSTER_DOWN	4
#define CLUSTER_SUSPEND	5

void trace_power_flow(unsigned long mpidr, unsigned char mode);

#endif /* POWER_TRACER_H */
