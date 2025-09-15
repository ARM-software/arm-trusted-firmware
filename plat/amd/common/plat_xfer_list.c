/*
 * Copyright (c) 2023-2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <transfer_list.h>

static struct transfer_list_header *tl_hdr;
static int32_t tl_ops_holder;

bool populate_data_from_xfer_list(void)
{
	bool ret = true;

	tl_hdr = (struct transfer_list_header *)FW_HANDOFF_BASE;
	tl_ops_holder = transfer_list_check_header(tl_hdr);

	if ((tl_ops_holder != TL_OPS_ALL) && (tl_ops_holder != TL_OPS_RO)) {
		ret = false;
	}

	return ret;
}

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33)
{
	int32_t ret = tl_ops_holder;
	struct transfer_list_entry *te = NULL;
	struct entry_point_info *ep = NULL;

	if ((tl_ops_holder == TL_OPS_ALL) || (tl_ops_holder == TL_OPS_RO)) {
		transfer_list_dump(tl_hdr);
		while ((te = transfer_list_next(tl_hdr, te)) != NULL) {
			ep = transfer_list_entry_data(te);
			if (te->tag_id == TL_TAG_EXEC_EP_INFO64) {
				switch (GET_SECURITY_STATE(ep->h.attr)) {
				case NON_SECURE:
					*bl33 = *ep;
					break;
				case SECURE:
					*bl32 = *ep;
#if defined(SPD_opteed)
					/*
					 * Populate the args expected by opteed,
					 * arg0 - dt address,
					 * arg1 - Xfer List Convention Version,
					 * arg3 - Xfer List address
					 * remaining args are set to 0.
					 */
					if (transfer_list_set_handoff_args(tl_hdr, bl32) == NULL) {
						ERROR("Invalid transfer list\n");
					}
#endif /* SPD_opteed */
					break;
				default:
					ERROR("Unrecognized Image Security State %lu\n",
					      GET_SECURITY_STATE(ep->h.attr));
					ret = TL_OPS_NON;
				}
				/*
				 * Clearing the transfer list handoff entry data.
				 */
				memset(ep, 0, te->data_size);

				if (transfer_list_rem(tl_hdr, te) == false) {
					INFO("Failed to remove handoff info\n");
				}
			}
		}
	}

	return ret;
}

void *transfer_list_retrieve_dt_address(void)
{
	void *dtb = NULL;
	struct transfer_list_entry *te = NULL;

	if ((tl_ops_holder == TL_OPS_ALL) || (tl_ops_holder == TL_OPS_RO)) {
		te = transfer_list_find(tl_hdr, TL_TAG_FDT);
		if (te != NULL) {
			dtb = transfer_list_entry_data(te);
		}
	}

	return dtb;
}
