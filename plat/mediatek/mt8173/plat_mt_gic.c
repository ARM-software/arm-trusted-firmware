/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <lib/utils.h>

#include <mt8173_def.h>

const unsigned int mt_irq_sec_array[] = {
	MT_IRQ_SEC_SGI_0,
	MT_IRQ_SEC_SGI_1,
	MT_IRQ_SEC_SGI_2,
	MT_IRQ_SEC_SGI_3,
	MT_IRQ_SEC_SGI_4,
	MT_IRQ_SEC_SGI_5,
	MT_IRQ_SEC_SGI_6,
	MT_IRQ_SEC_SGI_7
};

void plat_mt_gic_init(void)
{
	arm_gic_init(BASE_GICC_BASE,
		BASE_GICD_BASE,
		BASE_GICR_BASE,
		mt_irq_sec_array,
		ARRAY_SIZE(mt_irq_sec_array));
}
