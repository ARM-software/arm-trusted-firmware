/*
 * Copyright (c) 2023-2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_XFER_LIST_H
#define PLAT_XFER_LIST_H

#include <transfer_list.h>

/* 8-byte subtype prepended to the DTBO blob in TL_TAG_FDT_OVRLY entry */
#define TL_FDT_OVRLY_SUBTYPE_SIZE	sizeof(uint64_t)
#define TL_FDT_OVRLY_SUBTYPE		0ULL

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33);

void *transfer_list_retrieve_dt_address(uint32_t tag_id);
bool init_transfer_list_from_fdt_or_static(void);
int32_t tl_init_ns_transfer_list(entry_point_info_t *bl33_image);
int32_t tl_populate_ns_transfer_list(void);
int32_t tl_add_dt_overlay(void *fdt, size_t fdtsize);

#endif /* PLAT_XFER_LIST_H */
