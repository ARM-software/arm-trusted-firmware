/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_PRIVATE_H
#define AMU_PRIVATE_H

#include <stdint.h>

uint64_t amu_group0_cnt_read_internal(int idx);
void amu_group0_cnt_write_internal(int idx, uint64_t val);

uint64_t amu_group1_cnt_read_internal(int idx);
void amu_group1_cnt_write_internal(int idx, uint64_t val);
void amu_group1_set_evtype_internal(int idx, unsigned int val);

#endif /* AMU_PRIVATE_H */
