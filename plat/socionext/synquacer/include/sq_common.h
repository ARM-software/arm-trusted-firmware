/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SQ_COMMON_H__
#define __SQ_COMMON_H__

#include <sys/types.h>

void plat_sq_interconnect_init(void);
void plat_sq_interconnect_enter_coherency(void);
void plat_sq_interconnect_exit_coherency(void);

unsigned int sq_calc_core_pos(u_register_t mpidr);

void sq_gic_driver_init(void);
void sq_gic_init(void);
void sq_gic_cpuif_enable(void);
void sq_gic_cpuif_disable(void);
void sq_gic_pcpu_init(void);

#endif /* __SQ_COMMON_H__ */
