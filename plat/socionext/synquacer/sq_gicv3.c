/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <plat/common/platform.h>

#include "sq_common.h"

static uintptr_t sq_rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t sq_interrupt_props[] = {
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

static unsigned int sq_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const struct gicv3_driver_data sq_gic_driver_data = {
		.gicd_base = PLAT_SQ_GICD_BASE,
		.gicr_base = PLAT_SQ_GICR_BASE,
		.interrupt_props = sq_interrupt_props,
		.interrupt_props_num = ARRAY_SIZE(sq_interrupt_props),
		.rdistif_num = PLATFORM_CORE_COUNT,
		.rdistif_base_addrs = sq_rdistif_base_addrs,
		.mpidr_to_core_pos = sq_mpidr_to_core_pos,
};

void sq_gic_driver_init(void)
{
	gicv3_driver_init(&sq_gic_driver_data);
}

void sq_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void sq_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void sq_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void sq_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}
