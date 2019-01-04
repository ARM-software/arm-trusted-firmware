/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/desc_image_load.h>
#include <plat/common/platform.h>

/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {
#ifdef EL3_PAYLOAD_BASE
	/* Fill EL3 payload related information (BL31 is EL3 payload) */
	{ .image_id = BL31_IMAGE_ID,

	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t,
				SECURE | EXECUTABLE | EP_FIRST_EXE),
	  .ep_info.pc = EL3_PAYLOAD_BASE,
	  .ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
				  DISABLE_ALL_EXCEPTIONS),

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				IMAGE_ATTRIB_PLAT_SETUP | IMAGE_ATTRIB_SKIP_LOADING),

	  .next_handoff_image_id = INVALID_IMAGE_ID,
	},
#else /* EL3_PAYLOAD_BASE */
#ifdef AARCH64
	/* Fill BL31 related information */
	{ .image_id = BL31_IMAGE_ID,

	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t,
				SECURE | EXECUTABLE | EP_FIRST_EXE),
	  .ep_info.pc = BL31_BASE,
	  .ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
				  DISABLE_ALL_EXCEPTIONS),
# if DEBUG
	  .ep_info.args.arg1 = QEMU_BL31_PLAT_PARAM_VAL,
# endif
	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				IMAGE_ATTRIB_PLAT_SETUP),
	  .image_info.image_base = BL31_BASE,
	  .image_info.image_max_size = BL31_LIMIT - BL31_BASE,

# ifdef QEMU_LOAD_BL32
	  .next_handoff_image_id = BL32_IMAGE_ID,
# else
	  .next_handoff_image_id = BL33_IMAGE_ID,
# endif
	},
#endif /* AARCH64 */
# ifdef QEMU_LOAD_BL32

#ifdef AARCH64
#define BL32_EP_ATTRIBS		(SECURE | EXECUTABLE)
#define BL32_IMG_ATTRIBS	0
#else
#define BL32_EP_ATTRIBS		(SECURE | EXECUTABLE | EP_FIRST_EXE)
#define BL32_IMG_ATTRIBS	IMAGE_ATTRIB_PLAT_SETUP
#endif

	/* Fill BL32 related information */
	{ .image_id = BL32_IMAGE_ID,

	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, BL32_EP_ATTRIBS),
	  .ep_info.pc = BL32_BASE,

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, BL32_IMG_ATTRIBS),

	  .image_info.image_base = BL32_BASE,
	  .image_info.image_max_size = BL32_LIMIT - BL32_BASE,

	  .next_handoff_image_id = BL33_IMAGE_ID,
	},

	/*
	 * Fill BL32 external 1 related information.
	 * A typical use for extra1 image is with OP-TEE where it is the
	 * pager image.
	 */
	{ .image_id = BL32_EXTRA1_IMAGE_ID,

	   SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				 entry_point_info_t, SECURE | NON_EXECUTABLE),

	   SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				 image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
	   .image_info.image_base = BL32_BASE,
	   .image_info.image_max_size = BL32_LIMIT - BL32_BASE,

	   .next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/*
	 * Fill BL32 external 2 related information.
	 * A typical use for extra2 image is with OP-TEE where it is the
	 * paged image.
	 */
	{ .image_id = BL32_EXTRA2_IMAGE_ID,

	   SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				 entry_point_info_t, SECURE | NON_EXECUTABLE),

	   SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				 image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
#if defined(SPD_opteed) || defined(AARCH32_SP_OPTEE)
	   .image_info.image_base = QEMU_OPTEE_PAGEABLE_LOAD_BASE,
	   .image_info.image_max_size = QEMU_OPTEE_PAGEABLE_LOAD_SIZE,
#endif
	   .next_handoff_image_id = INVALID_IMAGE_ID,
	},
# endif /* QEMU_LOAD_BL32 */

	/* Fill BL33 related information */
	{ .image_id = BL33_IMAGE_ID,
	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | EXECUTABLE),
# ifdef PRELOADED_BL33_BASE
	  .ep_info.pc = PRELOADED_BL33_BASE,

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				IMAGE_ATTRIB_SKIP_LOADING),
# else /* PRELOADED_BL33_BASE */
	  .ep_info.pc = NS_IMAGE_OFFSET,

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				0),
	  .image_info.image_base = NS_IMAGE_OFFSET,
	  .image_info.image_max_size = NS_DRAM0_BASE + NS_DRAM0_SIZE -
				       NS_IMAGE_OFFSET,
# endif /* !PRELOADED_BL33_BASE */

	  .next_handoff_image_id = INVALID_IMAGE_ID,
	}
#endif /* !EL3_PAYLOAD_BASE */
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
