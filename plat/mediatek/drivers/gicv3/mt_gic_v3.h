/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_GIC_V3_H
#define MT_GIC_V3_H

#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>

int32_t mt_irq_get_pending(uint32_t irq);
int32_t mt_irq_set_pending(uint32_t irq);
uint32_t gicr_get_sgi_pending(void);

void mt_gic_pcpu_init(void);
void mt_gic_distif_save(void);
void mt_gic_distif_restore(void);
void mt_gic_rdistif_save(void);
void mt_gic_rdistif_restore(void);
void mt_gic_redistif_on(void);
void mt_gic_redistif_off(void);
void mt_gic_redistif_init(void);
void mt_gic_cpuif_enable(void);
void mt_gic_cpuif_disable(void);
void mt_gic_driver_init(void);
void mt_gic_init(void);

#endif /* MT_GIC_V3_H */
