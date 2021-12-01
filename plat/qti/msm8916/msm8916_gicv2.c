/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>

#include "msm8916_gicv2.h"
#include <msm8916_mmap.h>

#define IRQ_SEC_SGI_0		8
#define IRQ_SEC_SGI_1		9
#define IRQ_SEC_SGI_2		10
#define IRQ_SEC_SGI_3		11
#define IRQ_SEC_SGI_4		12
#define IRQ_SEC_SGI_5		13
#define IRQ_SEC_SGI_6		14
#define IRQ_SEC_SGI_7		15

#define IRQ_SEC_PHY_TIMER	(16 + 2)	/* PPI #2 */

static const interrupt_prop_t msm8916_interrupt_props[] = {
	INTR_PROP_DESC(IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
};

static const gicv2_driver_data_t msm8916_gic_data = {
	.gicd_base		= APCS_QGIC2_GICD,
	.gicc_base		= APCS_QGIC2_GICC,
	.interrupt_props	= msm8916_interrupt_props,
	.interrupt_props_num	= ARRAY_SIZE(msm8916_interrupt_props),
};

void msm8916_gicv2_init(void)
{
	gicv2_driver_init(&msm8916_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}
