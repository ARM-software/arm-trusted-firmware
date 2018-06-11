/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_IMX8_H__
#define __PLAT_IMX8_H__

#include <gicv3.h>

unsigned int plat_calc_core_pos(uint64_t mpidr);
void imx_mailbox_init(uintptr_t base_addr);
void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_pcpu_init(void);

#endif /*__PLAT_IMX8_H__ */
