/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPCDMA_H
#define GPCDMA_H

#include <stdint.h>

void tegra_gpcdma_memcpy(uint64_t dst_addr, uint64_t src_addr,
			    uint32_t num_bytes);
void tegra_gpcdma_zeromem(uint64_t dst_addr, uint32_t num_bytes);

#endif /* GPCDMA_H */
