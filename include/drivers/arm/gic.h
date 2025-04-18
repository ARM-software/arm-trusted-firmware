/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef GIC_H
#define GIC_H
/* the function names conflict with some platform implementations. */
#if USE_GIC_DRIVER
void gic_init(unsigned int cpu_idx);
void gic_cpuif_enable(unsigned int cpu_idx);
void gic_cpuif_disable(unsigned int cpu_idx);
void gic_pcpu_off(unsigned int cpu_idx);
void gic_pcpu_init(unsigned int cpu_idx);
void gic_save(void);
void gic_resume(void);
#endif
#endif /* GIC_H */
