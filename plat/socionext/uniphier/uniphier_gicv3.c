/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <gicv3.h>
#include <platform.h>
#include <platform_def.h>

#include "uniphier.h"

static uintptr_t uniphier_rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const unsigned int g0_interrupt_array[] = {
	8,	/* SGI0 */
	14,	/* SGI6 */
};

static const unsigned int g1s_interrupt_array[] = {
	29,	/* Timer */
	9,	/* SGI1 */
	10,	/* SGI2 */
	11,	/* SGI3 */
	12,	/* SGI4 */
	13,	/* SGI5 */
	15,	/* SGI7 */
};

static unsigned int uniphier_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const struct gicv3_driver_data uniphier_gic_driver_data[] = {
	[UNIPHIER_SOC_LD11] = {
		.gicd_base = 0x5fe00000,
		.gicr_base = 0x5fe40000,
		.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
		.g1s_interrupt_num = ARRAY_SIZE(g1s_interrupt_array),
		.g0_interrupt_array = g0_interrupt_array,
		.g1s_interrupt_array = g1s_interrupt_array,
		.rdistif_num = PLATFORM_CORE_COUNT,
		.rdistif_base_addrs = uniphier_rdistif_base_addrs,
		.mpidr_to_core_pos = uniphier_mpidr_to_core_pos,
	},
	[UNIPHIER_SOC_LD20] = {
		.gicd_base = 0x5fe00000,
		.gicr_base = 0x5fe80000,
		.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
		.g1s_interrupt_num = ARRAY_SIZE(g1s_interrupt_array),
		.g0_interrupt_array = g0_interrupt_array,
		.g1s_interrupt_array = g1s_interrupt_array,
		.rdistif_num = PLATFORM_CORE_COUNT,
		.rdistif_base_addrs = uniphier_rdistif_base_addrs,
		.mpidr_to_core_pos = uniphier_mpidr_to_core_pos,
	},
	[UNIPHIER_SOC_PXS3] = {
		.gicd_base = 0x5fe00000,
		.gicr_base = 0x5fe80000,
		.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
		.g1s_interrupt_num = ARRAY_SIZE(g1s_interrupt_array),
		.g0_interrupt_array = g0_interrupt_array,
		.g1s_interrupt_array = g1s_interrupt_array,
		.rdistif_num = PLATFORM_CORE_COUNT,
		.rdistif_base_addrs = uniphier_rdistif_base_addrs,
		.mpidr_to_core_pos = uniphier_mpidr_to_core_pos,
	},
};

void uniphier_gic_driver_init(unsigned int soc)
{
	assert(soc < ARRAY_SIZE(uniphier_gic_driver_data));

	gicv3_driver_init(&uniphier_gic_driver_data[soc]);
}

void uniphier_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void uniphier_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void uniphier_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void uniphier_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}
