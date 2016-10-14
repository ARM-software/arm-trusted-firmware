/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __K3_GICV3_H__
#define __K3_GICV3_H__

void k3_gic_driver_init(uintptr_t gicd_base, uintptr_t gicr_base);
void k3_gic_init(void);
void k3_gic_cpuif_enable(void);
void k3_gic_cpuif_disable(void);
void k3_gic_pcpu_init(void);

#endif /* __K3_GICV3_H__ */
