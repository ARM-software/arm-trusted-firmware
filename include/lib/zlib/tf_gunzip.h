/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TF_GUNZIP_H
#define TF_GUNZIP_H

#include <stddef.h>
#include <stdint.h>

int gunzip(uintptr_t *in_buf, size_t in_len, uintptr_t *out_buf,
	   size_t out_len, uintptr_t work_buf, size_t work_len);

#endif /* TF_GUNZIP_H */
