/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_PRIVATE_H
#define AMU_PRIVATE_H

#include <stdint.h>

uint64_t amu_group0_cnt_read_internal(unsigned int idx);
void amu_group0_cnt_write_internal(unsigned int idx, uint64_t val);

uint64_t amu_group1_cnt_read_internal(unsigned int idx);
void amu_group1_cnt_write_internal(unsigned int idx, uint64_t val);
void amu_group1_set_evtype_internal(unsigned int idx, unsigned int val);

#if __aarch64__
uint64_t amu_group0_voffset_read_internal(unsigned int idx);
void amu_group0_voffset_write_internal(unsigned int idx, uint64_t val);

uint64_t amu_group1_voffset_read_internal(unsigned int idx);
void amu_group1_voffset_write_internal(unsigned int idx, uint64_t val);
#endif

#endif /* AMU_PRIVATE_H */
