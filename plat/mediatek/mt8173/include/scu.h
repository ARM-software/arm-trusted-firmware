/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCU_H
#define SCU_H

void disable_scu(unsigned long mpidr);
void enable_scu(unsigned long mpidr);

#endif /* SCU_H */
