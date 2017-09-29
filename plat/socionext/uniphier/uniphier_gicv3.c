/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <gicv3.h>
#include <interrupt_props.h>
#include <platform.h>
#include <platform_def.h>

#include "uniphier.h"

static uintptr_t uniphier_rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t uniphier_interrupt_props[] = {
	/* G0 interrupts */

	/* SGI0 */
	INTR_PROP_DESC(8, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
			GIC_INTR_CFG_EDGE),
	/* SGI6 */
	INTR_PROP_DESC(14, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP0,
			GIC_INTR_CFG_EDGE),

	/* G1S interrupts */

	/* Timer */
	INTR_PROP_DESC(29, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_LEVEL),
	/* SGI1 */
	INTR_PROP_DESC(9, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_EDGE),
	/* SGI2 */
	INTR_PROP_DESC(10, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_EDGE),
	/* SGI3 */
	INTR_PROP_DESC(11, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_EDGE),
	/* SGI4 */
	INTR_PROP_DESC(12, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_EDGE),
	/* SGI5 */
	INTR_PROP_DESC(13, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_EDGE),
	/* SGI7 */
	INTR_PROP_DESC(15, GIC_HIGHEST_SEC_PRIORITY, INTR_GROUP1S,
			GIC_INTR_CFG_EDGE)
};

static unsigned int uniphier_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const struct gicv3_driver_data uniphier_gic_driver_data[] = {
	[UNIPHIER_SOC_LD11] = {
		.gicd_base = 0x5fe00000,
		.gicr_base = 0x5fe40000,
		.interrupt_props = uniphier_interrupt_props,
		.interrupt_props_num = ARRAY_SIZE(uniphier_interrupt_props),
		.rdistif_num = PLATFORM_CORE_COUNT,
		.rdistif_base_addrs = uniphier_rdistif_base_addrs,
		.mpidr_to_core_pos = uniphier_mpidr_to_core_pos,
	},
	[UNIPHIER_SOC_LD20] = {
		.gicd_base = 0x5fe00000,
		.gicr_base = 0x5fe80000,
		.interrupt_props = uniphier_interrupt_props,
		.interrupt_props_num = ARRAY_SIZE(uniphier_interrupt_props),
		.rdistif_num = PLATFORM_CORE_COUNT,
		.rdistif_base_addrs = uniphier_rdistif_base_addrs,
		.mpidr_to_core_pos = uniphier_mpidr_to_core_pos,
	},
	[UNIPHIER_SOC_PXS3] = {
		.gicd_base = 0x5fe00000,
		.gicr_base = 0x5fe80000,
		.interrupt_props = uniphier_interrupt_props,
		.interrupt_props_num = ARRAY_SIZE(uniphier_interrupt_props),
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
