/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/desc_image_load.h>

#if CRYPTO_SUPPORT
#include <mbedtls/version.h>
#endif /* CRYPTO_SUPPORT */

#if BL2_ENABLE_SP_LOAD
#include <plat/arm/common/fconf_arm_sp_getter.h>
#endif /* BL2_ENABLE_SP_LOAD */
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#if CRYPTO_SUPPORT
#if defined(IMAGE_BL1) || RESET_TO_BL2 || defined(IMAGE_BL31)
static unsigned char heap[TF_MBEDTLS_HEAP_SIZE];

#define MBEDTLS_HEAP_ADDR heap
#define MBEDTLS_HEAP_SIZE sizeof(heap)
#else
static struct crypto_heap_info heap_info;

#define MBEDTLS_HEAP_ADDR heap_info.addr
#define MBEDTLS_HEAP_SIZE heap_info.size

struct transfer_list_entry *
arm_transfer_list_set_heap_info(struct transfer_list_header *tl)
{
	struct transfer_list_entry *te =
		transfer_list_find(tl, TL_TAG_MBEDTLS_HEAP_INFO);
	assert(te != NULL);

	heap_info = *(struct crypto_heap_info *)transfer_list_entry_data(te);
	return te;
}
#endif /* defined(IMAGE_BL1) || RESET_TO_BL2 || defined(IMAGE_BL31) */

int __init arm_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);
	*heap_addr = MBEDTLS_HEAP_ADDR;
	*heap_size = MBEDTLS_HEAP_SIZE;

	return 0;
}
#endif /* CRYPTO_SUPPORT */

void arm_transfer_list_dyn_cfg_init(struct transfer_list_header *tl)
{
	int err __maybe_unused;
	struct transfer_list_entry *te;
	bl_mem_params_node_t *next_param_node =
		get_bl_mem_params_node(HW_CONFIG_ID);
	assert(next_param_node != NULL);

	/*
	 * The HW_CONFIG needs to be authenticated via the normal loading
	 * mechanism. Pre-allocate a TE for the configuration and update the
	 * load information so the configuration is loaded directly into the TE.
	 */
	te = transfer_list_add(tl, TL_TAG_FDT, PLAT_ARM_HW_CONFIG_SIZE, NULL);
	assert(te != NULL);

	next_param_node->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	next_param_node->image_info.image_max_size = PLAT_ARM_HW_CONFIG_SIZE;
	next_param_node->image_info.image_base =
		(uintptr_t)transfer_list_entry_data(te);

#if BL2_ENABLE_SP_LOAD && defined(PLAT_ARM_TB_FW_CONFIG_TL_TAG)
	te = transfer_list_find(tl, PLAT_ARM_TB_FW_CONFIG_TL_TAG);
	assert(te != NULL);

	/*
	 * TB_FW_CONFIG is loaded in the TL_TAG_FDT entry at
	 * arm_bl31_platform_setup().
	 * This data will be clear after loading SP_PKGs information.
	 *
	 * Later, TL_TAG_FDT data will be loaded with HW_CONFIG at
	 * arm_bl31_platform_setup().
	 */
	err = fconf_populate_arm_sp((uintptr_t)transfer_list_entry_data(te));
	if (err) {
		ERROR("Failed to populate SP information. err: %d\n", err);
	}
#endif

#if defined(SPD_spmd) && defined(PLAT_ARM_SPMC_SP_MANIFEST_SIZE)
	next_param_node = get_bl_mem_params_node(TOS_FW_CONFIG_ID);
	if (next_param_node == NULL) {
		return;
	}

#if SPMC_AT_EL3
	te = transfer_list_add(tl, TL_TAG_DT_FFA_MANIFEST,
			PLAT_ARM_SPMC_SP_MANIFEST_SIZE, NULL);
#else
	te = transfer_list_add(tl, TL_TAG_DT_SPMC_MANIFEST,
			PLAT_ARM_SPMC_SP_MANIFEST_SIZE, NULL);
#endif
	assert(te != NULL);

	next_param_node->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	next_param_node->image_info.image_max_size = PLAT_ARM_SPMC_SP_MANIFEST_SIZE;
	next_param_node->image_info.image_base =
		(uintptr_t)transfer_list_entry_data(te);
#endif /* defined(SPD_spmd) && defined(PLAT_ARM_SPMC_SP_MANIFEST_SIZE) */
}

void arm_transfer_list_populate_ep_info(bl_mem_params_node_t *next_param_node,
					struct transfer_list_header *tl)
{
	uint32_t next_exe_img_id;
	entry_point_info_t *ep __unused;
	struct transfer_list_entry *te;
	assert(next_param_node != NULL);

	while ((next_exe_img_id = next_param_node->next_handoff_image_id) !=
	       INVALID_IMAGE_ID) {
		next_param_node =
			&bl_mem_params_desc_ptr[get_bl_params_node_index(
				next_exe_img_id)];
		assert(next_param_node != NULL);

		te = transfer_list_add(tl, TL_TAG_EXEC_EP_INFO,
				       sizeof(entry_point_info_t),
				       &next_param_node->ep_info);
		assert(te != NULL);

		ep = transfer_list_entry_data(te);
		assert(ep != NULL);

#if SPMC_AT_EL3
		if (next_exe_img_id == BL32_IMAGE_ID) {
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
#endif /* SPMC_AT_EL3 */

		next_exe_img_id = next_param_node->next_handoff_image_id;
	}

	flush_dcache_range((uintptr_t)tl, tl->size);
}
