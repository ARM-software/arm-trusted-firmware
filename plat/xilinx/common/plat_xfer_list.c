/*
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/transfer_list.h>

/*
 * FIXME: This address should come from firmware before TF-A runs
 * Having this to make sure the transfer list functionality works
 */
#define FW_HANDOFF_BASE		U(0x1200000)
#define FW_HANDOFF_SIZE		U(0x600000)

static struct transfer_list_header *tl_hdr;

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33)
{
	struct transfer_list_entry *te = NULL;
	struct entry_point_info *ep;
	int32_t ret;

	tl_hdr = (struct transfer_list_header *)FW_HANDOFF_BASE;
	ret = transfer_list_check_header(tl_hdr);
	if ((ret == TL_OPS_ALL) || (ret == TL_OPS_RO)) {
		transfer_list_dump(tl_hdr);
		while ((te = transfer_list_next(tl_hdr, te)) != NULL) {
			ep = transfer_list_entry_data(te);
			if (te->tag_id == TL_TAG_EXEC_EP_INFO64) {
				switch (GET_SECURITY_STATE(ep->h.attr)) {
				case NON_SECURE:
					*bl33 = *ep;
					continue;
				case SECURE:
					*bl32 = *ep;
					continue;
				default:
					ERROR("Unrecognized Image Security State %lu\n",
					      GET_SECURITY_STATE(ep->h.attr));
					ret = TL_OPS_NON;
				}
			}
		}
	}
	return ret;
}
