/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TF_CRC32_H
#define TF_CRC32_H

#include <stddef.h>
#include <stdint.h>

/* compute CRC using Arm intrinsic function */
uint32_t tf_crc32(uint32_t crc, const unsigned char *buf, size_t size);

#endif /* TF_CRC32_H */
