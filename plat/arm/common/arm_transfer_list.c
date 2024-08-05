/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

void arm_transfer_list_dyn_cfg_init(struct transfer_list_header *secure_tl)
{
	struct transfer_list_entry *te;
	bl_mem_params_node_t *next_param_node =
		get_bl_mem_params_node(HW_CONFIG_ID);
	assert(next_param_node != NULL);

	/*
	 * The HW_CONFIG needs to be authenticated via the normal loading
	 * mechanism. Pre-allocate a TE for the configuration and update the
	 * load information so the configuration is loaded directly into the TE.
	 */
	te = transfer_list_add(secure_tl, TL_TAG_FDT, PLAT_ARM_HW_CONFIG_SIZE,
			       NULL);
	assert(te != NULL);

	next_param_node->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	next_param_node->image_info.image_max_size = PLAT_ARM_HW_CONFIG_SIZE;
	next_param_node->image_info.image_base =
		(uintptr_t)transfer_list_entry_data(te);
}

void arm_transfer_list_populate_ep_info(bl_mem_params_node_t *next_param_node,
					struct transfer_list_header *secure_tl)
{
	uint32_t next_exe_img_id;
	entry_point_info_t *ep;
	struct transfer_list_entry *te;

	assert(next_param_node != NULL);

	while ((next_exe_img_id = next_param_node->next_handoff_image_id) !=
	       INVALID_IMAGE_ID) {
		next_param_node =
			&bl_mem_params_desc_ptr[get_bl_params_node_index(
				next_exe_img_id)];
		assert(next_param_node != NULL);

		te = transfer_list_add(secure_tl, TL_TAG_EXEC_EP_INFO64,
				       sizeof(entry_point_info_t),
				       &next_param_node->ep_info);
		assert(te != NULL);

		ep = transfer_list_entry_data(te);

		if ((next_exe_img_id == BL32_IMAGE_ID) && SPMC_AT_EL3) {
			/*
			 * Populate the BL32 image base, size and max limit in
			 * the entry point information, since there is no
			 * platform function to retrieve them in generic
			 * code. We choose arg2, arg3 and arg4 since the generic
			 * code uses arg1 for stashing the SP manifest size. The
			 * SPMC setup uses these arguments to update SP manifest
			 * with actual SP's base address and it size.
			 */
			ep->args.arg2 = next_param_node->image_info.image_base;
			ep->args.arg3 = next_param_node->image_info.image_size;
			ep->args.arg4 =
				next_param_node->image_info.image_base +
				next_param_node->image_info.image_max_size;
		}

		next_exe_img_id = next_param_node->next_handoff_image_id;
	}

	flush_dcache_range((uintptr_t)secure_tl, secure_tl->size);
}
