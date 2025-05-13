/*
 * Copyright (c) 2023-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_XFER_LIST_H
#define PLAT_XFER_LIST_H

#include <transfer_list.h>

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33);

void *transfer_list_retrieve_dt_address(void);
bool populate_data_from_xfer_list(void);

#endif /* PLAT_XFER_LIST_H */
