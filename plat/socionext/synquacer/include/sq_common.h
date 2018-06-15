/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SQ_COMMON_H__
#define __SQ_COMMON_H__

#include <sys/types.h>

unsigned int sq_calc_core_pos(u_register_t mpidr);

#endif /* __SQ_COMMON_H__ */
