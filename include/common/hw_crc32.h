/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HW_CRC32_H
#define HW_CRC32_H

#include <stddef.h>
#include <stdint.h>

/* compute CRC using Arm intrinsic function */
uint32_t hw_crc32(uint32_t crc, const unsigned char *buf, size_t size);

#endif /* HW_CRC32_H */
