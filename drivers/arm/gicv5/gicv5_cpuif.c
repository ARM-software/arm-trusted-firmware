/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/gic.h>
#include <drivers/arm/gicv5.h>

void gic_cpuif_enable(unsigned int cpu_idx)
{
}

void gic_cpuif_disable(unsigned int cpu_idx)
{
}

void gic_pcpu_init(unsigned int cpu_idx)
{
	gicv5_enable_ppis();
}

void gic_pcpu_off(unsigned int cpu_idx)
{
}

