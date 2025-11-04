/*
 * Copyright (c) 2023-2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_XFER_LIST_H
#define PLAT_XFER_LIST_H

#include <transfer_list.h>

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33);

void *transfer_list_retrieve_dt_address(void);
bool init_transfer_list_from_fdt_or_static(void);
int32_t tl_init_ns_transfer_list(entry_point_info_t *bl33_image);
int32_t tl_populate_ns_transfer_list(void);

#endif /* PLAT_XFER_LIST_H */
