/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <ti_host_idx_mapping.h>
#include <ti_hosts.h>

const uint8_t soc_host_indexes[1] = {
	[HOST_ID_TIFS] = 0U,
};
const uint8_t soc_host_indexes_count = (uint8_t)sizeof(soc_host_indexes);
