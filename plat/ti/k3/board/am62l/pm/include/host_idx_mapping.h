/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SOC_HOST_IDX_MAPPING_H
#define SOC_HOST_IDX_MAPPING_H

#include <types/short_types.h>
#include <clock_limits.h>

#define HOST_IDX_NONE 255U

extern const uint8_t soc_host_indexes[HOST_RANGE_ID_MAX];
extern const uint8_t soc_host_indexes_sz;

static inline uint8_t host_idx_lookup(uint8_t host_id)
{
	return (host_id < soc_host_indexes_sz) ? soc_host_indexes[host_id] : HOST_IDX_NONE;
}

#endif
