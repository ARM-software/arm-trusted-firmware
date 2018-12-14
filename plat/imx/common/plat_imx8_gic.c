/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <plat_imx8.h>

/* the GICv3 driver only needs to be initialized in EL3 */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t g01s_interrupt_props[] = {
	INTR_PROP_DESC(6, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP1S, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(7, GIC_HIGHEST_SEC_PRIORITY,
		       INTR_GROUP0, GIC_INTR_CFG_LEVEL),
};

static unsigned int plat_imx_mpidr_to_core_pos(unsigned long mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

const gicv3_driver_data_t arm_gic_data = {
	.gicd_base = PLAT_GICD_BASE,
	.gicr_base = PLAT_GICR_BASE,
	.interrupt_props = g01s_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(g01s_interrupt_props),
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_imx_mpidr_to_core_pos,
};

void plat_gic_driver_init(void)
{
	/*
	 * the GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in S-EL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if IMAGE_BL31
	gicv3_driver_init(&arm_gic_data);
#endif
}

void plat_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void plat_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void plat_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void plat_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}
