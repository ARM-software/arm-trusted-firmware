/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AMU_PRIVATE_H__
#define __AMU_PRIVATE_H__

#include <stdint.h>

uint64_t amu_group0_cnt_read_internal(int idx);
void amu_group0_cnt_write_internal(int idx, uint64_t);

uint64_t amu_group1_cnt_read_internal(int idx);
void amu_group1_cnt_write_internal(int idx, uint64_t);
void amu_group1_set_evtype_internal(int idx, unsigned int val);

#endif /* __AMU_PRIVATE_H__ */
