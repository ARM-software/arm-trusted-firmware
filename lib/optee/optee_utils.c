/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <lib/optee_utils.h>

/*
 * load_addr_hi and load_addr_lo: image load address.
 * image_id: 0 - pager, 1 - paged
 * size: image size in bytes.
 */
typedef struct optee_image {
	uint32_t load_addr_hi;
	uint32_t load_addr_lo;
	uint32_t image_id;
	uint32_t size;
} optee_image_t;

#define OPTEE_PAGER_IMAGE_ID		0
#define OPTEE_PAGED_IMAGE_ID		1

#define OPTEE_MAX_NUM_IMAGES		2u

#define TEE_MAGIC_NUM_OPTEE		0x4554504f
/*
 * magic: header magic number.
 * version: OPTEE header version:
 *		1 - not supported
 *		2 - supported
 * arch: OPTEE os architecture type: 0 - AARCH32, 1 - AARCH64.
 * flags: unused currently.
 * nb_images: number of images.
 */
typedef struct optee_header {
	uint32_t magic;
	uint8_t version;
	uint8_t arch;
	uint16_t flags;
	uint32_t nb_images;
	optee_image_t optee_image_list[];
} optee_header_t;

/*******************************************************************************
 * Check if it is a valid tee header
 * Return 1 if valid
 * Return 0 if invalid
 ******************************************************************************/
static inline int tee_validate_header(optee_header_t *header)
{
	int valid = 0;

	if ((header->magic == TEE_MAGIC_NUM_OPTEE) &&
		(header->version == 2u) &&
		(header->nb_images > 0u) &&
		(header->nb_images <= OPTEE_MAX_NUM_IMAGES)) {
		valid = 1;
	}

	else {
		WARN("Not a known TEE, use default loading options.\n");
	}

	return valid;
}

/*******************************************************************************
 * Parse the OPTEE image
 * Return 0 on success or a negative error code otherwise.
 ******************************************************************************/
static int parse_optee_image(image_info_t *image_info,
		optee_image_t *image)
{
	uintptr_t init_load_addr, free_end, requested_end;
	size_t init_size;

	init_load_addr = ((uint64_t)image->load_addr_hi << 32) |
					image->load_addr_lo;
	init_size = image->size;

	/*
	 * -1 indicates loader decided address; take our pre-mapped area
	 * for current image since arm-tf could not allocate memory dynamically
	 */
	if (init_load_addr == -1)
		init_load_addr = image_info->image_base;

	/* Check that the default end address doesn't overflow */
	if (check_uptr_overflow(image_info->image_base,
				image_info->image_max_size - 1))
		return -1;
	free_end = image_info->image_base + (image_info->image_max_size - 1);

	/* Check that the image end address doesn't overflow */
	if (check_uptr_overflow(init_load_addr, init_size - 1))
		return -1;
	requested_end = init_load_addr + (init_size - 1);
	/*
	 * Check that the requested RAM location is within reserved
	 * space for OPTEE.
	 */
	if (!((init_load_addr >= image_info->image_base) &&
			(requested_end <= free_end))) {
		WARN("The load address in optee header %p - %p is not in reserved area: %p - %p.\n",
				(void *)init_load_addr,
				(void *)(init_load_addr + init_size),
				(void *)image_info->image_base,
				(void *)(image_info->image_base +
					image_info->image_max_size));
		return -1;
	}

	/*
	 * Remove the skip attr from image_info, the image will be loaded.
	 * The default attr in image_info is "IMAGE_ATTRIB_SKIP_LOADING", which
	 * mean the image will not be loaded. Here, we parse the header image to
	 * know that the extra image need to be loaded, so remove the skip attr.
	 */
	image_info->h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;

	/* Update image base and size of image_info */
	image_info->image_base = init_load_addr;
	image_info->image_size = init_size;

	return 0;
}

/*******************************************************************************
 * Parse the OPTEE header
 * Return 0 on success or a negative error code otherwise.
 ******************************************************************************/
int parse_optee_header(entry_point_info_t *header_ep,
		image_info_t *pager_image_info,
		image_info_t *paged_image_info)

{
	optee_header_t *header;
	int num, ret;

	assert(header_ep);
	header = (optee_header_t *)header_ep->pc;
	assert(header);

	/* Print the OPTEE header information */
	INFO("OPTEE ep=0x%x\n", (unsigned int)header_ep->pc);
	INFO("OPTEE header info:\n");
	INFO("      magic=0x%x\n", header->magic);
	INFO("      version=0x%x\n", header->version);
	INFO("      arch=0x%x\n", header->arch);
	INFO("      flags=0x%x\n", header->flags);
	INFO("      nb_images=0x%x\n", header->nb_images);

	/*
	 * OPTEE image has 3 types:
	 *
	 * 1. Plain OPTEE bin without header.
	 *	Original bin without header, return directly,
	 *	BL32_EXTRA1_IMAGE_ID and BL32_EXTRA2_IMAGE_ID will be skipped.
	 *
	 * 2. OPTEE bin with header bin, but no paging.
	 *	Header available and nb_images = 1, remove skip attr for
	 *	BL32_EXTRA1_IMAGE_ID. BL32_EXTRA1_IMAGE_ID will be loaded,
	 *	and BL32_EXTRA2_IMAGE_ID be skipped.
	 *
	 * 3. OPTEE image with paging support.
	 *	Header available and nb_images = 2, there are 3 bins: header,
	 *	pager and pageable. Remove skip attr for BL32_EXTRA1_IMAGE_ID
	 *	and BL32_EXTRA2_IMAGE_ID to load pager and paged bin.
	 */
	if (!tee_validate_header(header)) {
		INFO("Invalid OPTEE header, set legacy mode.\n");
#ifdef AARCH64
		header_ep->args.arg0 = MODE_RW_64;
#else
		header_ep->args.arg0 = MODE_RW_32;
#endif
		return 0;
	}

	/* Parse OPTEE image */
	for (num = 0; num < header->nb_images; num++) {
		if (header->optee_image_list[num].image_id ==
				OPTEE_PAGER_IMAGE_ID) {
			ret = parse_optee_image(pager_image_info,
				&header->optee_image_list[num]);
		} else if (header->optee_image_list[num].image_id ==
				OPTEE_PAGED_IMAGE_ID) {
			ret = parse_optee_image(paged_image_info,
				&header->optee_image_list[num]);
		} else {
			ERROR("Parse optee image failed.\n");
			return -1;
		}

		if (ret != 0)
			return -1;
	}

	/*
	 * Update "pc" value which should comes from pager image. After the
	 * header image is parsed, it will be unuseful, and the actual
	 * execution image after BL31 is pager image.
	 */
	header_ep->pc =	pager_image_info->image_base;

	/*
	 * The paged load address and size are populated in
	 * header image arguments so that can be read by the
	 * BL32 SPD.
	 */
	header_ep->args.arg1 = paged_image_info->image_base;
	header_ep->args.arg2 = paged_image_info->image_size;

	/* Set OPTEE runtime arch - aarch32/aarch64 */
	if (header->arch == 0) {
		header_ep->args.arg0 = MODE_RW_32;
	} else {
#ifdef AARCH64
		header_ep->args.arg0 = MODE_RW_64;
#else
		ERROR("Cannot boot an AArch64 OP-TEE\n");
		return -1;
#endif
	}

	return 0;
}
