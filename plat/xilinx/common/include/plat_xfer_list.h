/*
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_XFER_LIST_H
#define PLAT_XFER_LIST_H

#include <lib/transfer_list.h>

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33);

#endif /* PLAT_XFER_LIST_H */
