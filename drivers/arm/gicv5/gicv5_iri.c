/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <drivers/arm/gic.h>
#include <drivers/arm/gicv5.h>

#if USE_GIC_DRIVER != 5
#error "This file should only be used with USE_GIC_DRIVER=5"
#endif

void __init gic_init(unsigned int cpu_idx)
{
	gicv5_driver_init();
}

void gic_save(void)
{
}

void gic_resume(void)
{
}
