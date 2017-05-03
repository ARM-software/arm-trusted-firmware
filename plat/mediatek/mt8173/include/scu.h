/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SCU_H__
#define __SCU_H__

void disable_scu(unsigned long mpidr);
void enable_scu(unsigned long mpidr);

#endif
