/*
 * Copyright (c) 2023-2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>

#include <libfdt.h>
#include <transfer_list.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat_arm.h>

#include <plat_fdt.h>
#include <platform_def.h>

static struct transfer_list_header *tl_hdr;
static struct transfer_list_header *ns_tl_hdr;
static int32_t tl_ops_holder;

struct xfer_list_region {
	uintptr_t base;
	size_t size;
	bool is_mapped;
};

/* Secure and Non-Secure transfer list region */
static struct xfer_list_region secure_tl_region = {0};
static struct xfer_list_region ns_tl_region = {0};

static int32_t map_xfer_list_region(struct xfer_list_region *region, uint32_t attr)
{
	int32_t ret = -1;

	if (region == NULL) {
		ERROR("Invalid TL region\n");
		goto end;
	}

	if (region->is_mapped) {
		VERBOSE("Region is already mapped\n");
		ret = 0;
		goto end;
	}

	ret = mmap_add_dynamic_region((unsigned long long)region->base,
				      region->base,
				      region->size,
				      attr);

	if (ret != 0) {
		region->is_mapped = false;
		ERROR("Failed to map region at 0x%lx\n", (unsigned long)region->base);
	} else {
		region->is_mapped = true;
	}

end:
	return ret;
}

static int32_t unmap_xfer_list_region(struct xfer_list_region *region)
{
	int32_t ret = -1;

	if (region == NULL) {
		ERROR("Invalid TL region\n");
		goto end;
	}

	if (!region->is_mapped) {
		VERBOSE("Region is not mapped\n");
		ret = 0;
		goto end;
	}

	ret = mmap_remove_dynamic_region(region->base, region->size);

	if (ret != 0) {
		ERROR("Failed to unmap region at 0x%lx\n", (unsigned long)region->base);
	} else {
		region->is_mapped = false;
	}

end:
	return ret;
}

static bool setup_dynamic_transfer_list(void)
{
	entry_point_info_t bl32_ep_info = {0};
	entry_point_info_t bl33_ep_info = {0};
	void *dtb = (void *)FW_HANDOFF_BASE;
	uint32_t __unused mfst_sz = 0U;
	void __unused *mfst_dtb;
	uint32_t dtb_sz = 0U;
	bool ret = false;

	/* Add 2MB MAX DT size offset for Dynamic TL */
	tl_hdr = transfer_list_init((void *)(FW_HANDOFF_BASE + XILINX_OF_BOARD_DTB_MAX_SIZE),
			(FW_HANDOFF_SIZE - XILINX_OF_BOARD_DTB_MAX_SIZE));
	if (tl_hdr == NULL) {
		NOTICE("Failed to initialize Transfer List at 0x%lx\n",
				(unsigned long)(FW_HANDOFF_BASE + XILINX_OF_BOARD_DTB_MAX_SIZE));
		goto exit_on_failure;
	}

	tl_ops_holder = transfer_list_check_header(tl_hdr);
	if (tl_ops_holder != TL_OPS_ALL) {
		WARN("Unexpected TL ops after init: %d\n", tl_ops_holder);
		goto exit_on_failure;
	}

	/* Add entry point for BL32 */
	SET_PARAM_HEAD(&bl32_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_ep_info.h.attr, SECURE);
	bl32_ep_info.pc = BL32_BASE;
	bl32_ep_info.spsr = arm_get_spsr(BL32_IMAGE_ID);

	if (!transfer_list_add(tl_hdr, TL_TAG_EXEC_EP_INFO64,
				sizeof(entry_point_info_t), &bl32_ep_info)) {
		WARN("Failed to add BL32 entry to TL\n");
		goto exit_on_failure;
	}

	/* Add entry point for BL33 */
	SET_PARAM_HEAD(&bl33_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl33_ep_info.h.attr, NON_SECURE);
	bl33_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_ep_info.spsr = (uint32_t)SPSR_64(MODE_EL2, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS);

	if (!transfer_list_add(tl_hdr, TL_TAG_EXEC_EP_INFO64,
				sizeof(entry_point_info_t), &bl33_ep_info)) {
		WARN("Failed to add BL33 entry to TL\n");
		goto exit_on_failure;
	}

	if (is_valid_dtb(dtb) < 0) {
		WARN("Invalid or corrupted DT\n");
		goto exit_on_failure;
	}

	dtb_sz = fdt_totalsize(dtb);
	if ((dtb_sz == 0U) || (transfer_list_add(tl_hdr,
				TL_TAG_FDT, dtb_sz, dtb) == NULL)) {
		WARN("Failed to add DT entry to TL\n");
		goto exit_on_failure;
	}

#if defined(SPD_spmd)
	mfst_dtb = (void *)(uintptr_t)SPMC_MANIFEST_DTB_ADDR;

	/* Validate SPMC manifest DTB */
	if (is_valid_dtb(mfst_dtb) < 0) {
		WARN("Invalid SPMC manifest DTB at 0x%lx\n",
				(unsigned long)SPMC_MANIFEST_DTB_ADDR);
	} else {
		mfst_sz = fdt_totalsize(mfst_dtb);

		/* Add SPMC manifest DT TL entry */
		if ((mfst_sz == 0U) ||
				(transfer_list_add(tl_hdr,
					TL_TAG_DT_SPMC_MANIFEST,
					mfst_sz,
					mfst_dtb) == NULL)) {
			WARN("Failed to add SPMC manifest to TL\n");
		}
	}
#endif /* SPD_spmd */
	ret = true;

exit_on_failure:
	return ret;
}

static bool validate_transfer_list_ops(void)
{
	bool ret = true;

	tl_hdr = (struct transfer_list_header *)FW_HANDOFF_BASE;
	tl_ops_holder = transfer_list_check_header(tl_hdr);

	if ((tl_ops_holder != TL_OPS_ALL) && (tl_ops_holder != TL_OPS_RO)) {
		ret = false;
	}

	return ret;
}

bool init_transfer_list_from_fdt_or_static(void)
{
	void *blob_magic_addr = (void *)FW_HANDOFF_BASE;
	bool ret = true;
	int32_t map_ret;

	/* Initialize secure transfer list region structure */
	secure_tl_region.base = FW_HANDOFF_BASE;
	secure_tl_region.size = FW_HANDOFF_SIZE;
	secure_tl_region.is_mapped = false;

	/* Map secure transfer list region */
	map_ret = map_xfer_list_region(&secure_tl_region, MT_MEMORY | MT_RW | MT_SECURE);

	if (map_ret != 0) {
		ret = false;
		goto exit_on;
	}

	/* If blob address is of type FDT, switch to dynamic TL */
	if (fdt_magic(blob_magic_addr) == FDT_MAGIC) {
		if (!setup_dynamic_transfer_list()) {
			NOTICE("Failed to create dynamic transfer list.\n");
			ret = false;
			goto unmap_tl;
		}
		goto exit_on;
	}

	if (!validate_transfer_list_ops()) {
		ret = false;
		goto unmap_tl;
	}
	goto exit_on;

unmap_tl:
	/* Unmap secure transfer list region on failure */
	map_ret = unmap_xfer_list_region(&secure_tl_region);
	if (map_ret != 0) {
		NOTICE("Failed to unmap secure TL on failure.\n");
	}

exit_on:
	return ret;
}

int32_t transfer_list_populate_ep_info(entry_point_info_t *bl32,
				       entry_point_info_t *bl33)
{
	struct transfer_list_entry *te = NULL;
	struct entry_point_info *ep = NULL;
	int32_t ret = tl_ops_holder;

	if (secure_tl_region.is_mapped && ((tl_ops_holder == TL_OPS_ALL) ||
				(tl_ops_holder == TL_OPS_RO))) {
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

	if (secure_tl_region.is_mapped && ((tl_ops_holder == TL_OPS_ALL) ||
				(tl_ops_holder == TL_OPS_RO))) {
		te = transfer_list_find(tl_hdr, TL_TAG_FDT);
		if (te != NULL) {
			dtb = transfer_list_entry_data(te);
		}
	}

	return dtb;
}

int32_t tl_init_ns_transfer_list(entry_point_info_t *bl33_image)
{
	struct transfer_list_entry *ns_te = NULL;
	struct transfer_list_entry *te = NULL;
	int32_t ret = 0;
	int32_t map_ret;

	/* Initialize non-secure transfer list region structure */
	ns_tl_region.base = NS_FW_HANDOFF_BASE;
	/* Reusing max secure TL size for the NS region */
	ns_tl_region.size = FW_HANDOFF_SIZE;
	ns_tl_region.is_mapped = false;

	if (bl33_image == NULL) {
		WARN("BL33 image info is NULL\n");
		ret = -1;
		goto out;
	}

	if (!secure_tl_region.is_mapped) {
		WARN("Secure TL region is not mapped\n");
		ret = -1;
		goto out;
	}

	/* Map non-secure TL range */
	map_ret = map_xfer_list_region(&ns_tl_region, MT_MEMORY | MT_RW | MT_NS);
	if (map_ret != 0) {
		WARN("Failed to map non-secure TL region\n");
		ret = -1;
		goto out;
	}

	/*
	 * Placeholder non-secure transfer list to be updated at runtime setup.
	 * next_image_entry runs prior to runtime setup where in non-secure args
	 * are updated. In order to account for valid non-secure TL entries, DT only
	 * transfer list is created.
	 */
	ns_tl_hdr = transfer_list_init((void *)(NS_FW_HANDOFF_BASE), FW_HANDOFF_SIZE);
	if (ns_tl_hdr == NULL) {
		WARN("Non-secure transfer list initialization failed\n");
		ret = -1;
		goto unmap_tl;
	}

	te = transfer_list_find(tl_hdr, TL_TAG_FDT);
	if (te == NULL) {
		WARN("Secure transfer list DT not found\n");
		ret = -1;
		goto unmap_tl;
	}

	ns_te = transfer_list_add(ns_tl_hdr, TL_TAG_FDT, te->data_size,
			transfer_list_entry_data(te));
	if (ns_te == NULL) {
		WARN("Failed to add DT to non-secure transfer list\n");
		ret = -1;
		goto unmap_tl;
	}

	/*
	 * Populate the args expected by BL33,
	 * arg0 - dt address,
	 * arg1 - Xfer List Convention Version,
	 * arg2 - reserved/0,
	 * arg3 - Xfer List address
	 * remaining args are set to 0.
	 */
	if (transfer_list_set_handoff_args(ns_tl_hdr, bl33_image) == NULL) {
		ret = -1;
		WARN("Failed to update handoff args based on non-secure TL\n");
	}

	goto out;

unmap_tl:
	/* Unmap non-secure transfer list region on failure */
	map_ret = unmap_xfer_list_region(&ns_tl_region);
	if (map_ret != 0) {
		WARN("Failed to unmap non-secure TL on failure\n");
	}

out:
	return ret;
}

int32_t tl_populate_ns_transfer_list(void)
{
	struct transfer_list_entry *ns_te = NULL;
	struct transfer_list_entry *te = NULL;
	int32_t map_ret;
	int32_t ret = 0;

	/* Check if secure and non-secure TL regions are mapped */
	if (!secure_tl_region.is_mapped || !ns_tl_region.is_mapped) {
		ERROR("Secure or NS TL region is not mapped\n");
		ret = -1;
		goto unmap_tl;
	}

	/* Invalidate cache to get updated tl post bl32 updates it */
	inv_dcache_range((uintptr_t)tl_hdr, tl_hdr->max_size);

#if defined(SPD_opteed) || defined(SPD_spmd)
	/*
	 * Find the post-BL32 DT in the secure TL and update the NS TL DT
	 * entry placed at init time.
	 */
	te = transfer_list_find(tl_hdr, TL_TAG_FDT);
	if (te == NULL) {
		WARN("Transfer list DT not found\n");
		ret = -1;
		goto unmap_tl;
	}

	ns_te = transfer_list_find(ns_tl_hdr, TL_TAG_FDT);
	if (ns_te == NULL) {
		WARN("NS transfer list DT entry not found\n");
		ret = -1;
		goto unmap_tl;
	}

	if (!transfer_list_set_data_size(ns_tl_hdr, ns_te, te->data_size)) {
		WARN("Failed to resize DT entry in NS TL\n");
		ret = -1;
		goto unmap_tl;
	}

	(void)memcpy(transfer_list_entry_data(ns_te),
		     transfer_list_entry_data(te), te->data_size);

	/* Update NS TL header and checksum */
	transfer_list_update_checksum(ns_tl_hdr);

	/* Restart iteration from the beginning of TL entries */
	te = NULL;

#endif /* SPD_opteed || SPD_spmd */

	/* Copy non-DT entries from secure TL to non-secure TL */
	while ((te = transfer_list_next(tl_hdr, te)) != NULL) {
		if ((te->tag_id != TL_TAG_FDT) && (te->tag_id != TL_TAG_EMPTY)) {
			ns_te = transfer_list_add(ns_tl_hdr, te->tag_id, te->data_size,
					transfer_list_entry_data(te));
			if (ns_te == NULL) {
				WARN("Failed to add overlay to ns TL\n");
				ret = -1;
				goto unmap_tl;
			}
		}
	}

	transfer_list_update_checksum(ns_tl_hdr);

	/* Flush NS TL to memory so BL33 in normal world sees the data */
	flush_dcache_range((uintptr_t)ns_tl_hdr, ns_tl_hdr->size);

unmap_tl:
	/* unmap non-secure TL region */
	map_ret = unmap_xfer_list_region(&ns_tl_region);
	if (map_ret != 0) {
		WARN("Failed to unmap non-secure TL region\n");
	}

	/* unmapping secure TL region */
	map_ret = unmap_xfer_list_region(&secure_tl_region);
	if (map_ret != 0) {
		WARN("Failed to unmap secure TL region\n");
	}

	return ret;
}

int32_t tl_add_dt_overlay(void *fdt, size_t fdtsize)
{
	struct transfer_list_entry *te = NULL;
	int32_t ret = 0;

	if ((fdt == NULL) || (fdtsize == 0U)) {
		ret = -1;
		goto exit_on;
	}

	if (secure_tl_region.is_mapped && (tl_ops_holder == TL_OPS_ALL)) {
		te = transfer_list_add(tl_hdr, TL_TAG_FDT_OVRLY, fdtsize, fdt);
		if (te == NULL) {
			ret = -1;
			WARN("Failed to add overlay entry to secure TL\n");
		} else {
			flush_dcache_range((uintptr_t)tl_hdr, tl_hdr->size);
		}
	} else {
		WARN("Failed due to no-mapping or wrong ops holder\n");
		ret = -1;
	}

exit_on:
	return ret;
}

