/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MTCMOS_H
#define MTCMOS_H

/*
 * This function will turn off all the little core's power except cpu 0. The
 * cores in cluster 0 are all powered when the system power on. The System
 * Power Manager (SPM) will do nothing if it found the core's power was on
 * during CPU_ON psci call.
 */
void mtcmos_little_cpu_off(void);
uint32_t mtcmos_non_cpu_ctrl(uint32_t on, uint32_t mtcmos_num);

#endif /* MTCMOS_H */
