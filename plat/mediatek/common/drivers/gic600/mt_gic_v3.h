/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_GIC_V3_H
#define MT_GIC_V3_H

#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>

void mt_gic_driver_init(void);
void mt_gic_init(void);
void mt_gic_set_pending(uint32_t irq);
void mt_gic_distif_save(void);
void mt_gic_distif_restore(void);
void mt_gic_rdistif_init(void);
void mt_gic_rdistif_save(void);
void mt_gic_rdistif_restore(void);
void mt_gic_rdistif_restore_all(void);
void gic_sgi_save_all(void);
void gic_sgi_restore_all(void);
uint32_t mt_irq_get_pending(uint32_t irq);
void mt_irq_set_pending(uint32_t irq);

#endif /* MT_GIC_V3_H */
