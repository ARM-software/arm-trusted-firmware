/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TI_HOST_IDX_MAPPING_H
#define TI_HOST_IDX_MAPPING_H

#include <stdint.h>

#define TI_HOST_IDX_NONE 255U

extern const uint8_t soc_host_indexes[];
extern const uint8_t soc_host_indexes_count;

static inline uint8_t ti_host_idx_lookup(uint8_t host_id)
{
	return (host_id < soc_host_indexes_count) ? soc_host_indexes[host_id] : TI_HOST_IDX_NONE;
}

#endif /* TI_HOST_IDX_MAPPING_H */
