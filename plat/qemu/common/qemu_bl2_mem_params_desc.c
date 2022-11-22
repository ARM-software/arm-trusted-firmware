/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/desc_image_load.h>
#include <plat/common/platform.h>

#define SP_PKG_ENTRY(id) \
	{ \
		.image_id = (id), \
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY, VERSION_2, \
				      entry_point_info_t, \
				      SECURE | NON_EXECUTABLE), \
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY, \
				      VERSION_2, image_info_t, \
				      IMAGE_ATTRIB_SKIP_LOADING), \
		.next_handoff_image_id = INVALID_IMAGE_ID, \
	}

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
#ifdef __aarch64__
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
#endif /* __aarch64__ */
# ifdef QEMU_LOAD_BL32

#ifdef __aarch64__
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
#if defined(SPD_opteed) || defined(AARCH32_SP_OPTEE) || defined(SPMC_OPTEE)
	   .image_info.image_base = QEMU_OPTEE_PAGEABLE_LOAD_BASE,
	   .image_info.image_max_size = QEMU_OPTEE_PAGEABLE_LOAD_SIZE,
#endif
	   .next_handoff_image_id = INVALID_IMAGE_ID,
	},

#if defined(SPD_spmd)
	/* Fill TOS_FW_CONFIG related information */
	{
	    .image_id = TOS_FW_CONFIG_ID,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_2, image_info_t, 0),
	    .image_info.image_base = TOS_FW_CONFIG_BASE,
	    .image_info.image_max_size = TOS_FW_CONFIG_LIMIT -
					 TOS_FW_CONFIG_BASE,
	    .next_handoff_image_id = INVALID_IMAGE_ID,
	},

#if SPMD_SPM_AT_SEL2
	/* Fill TB_FW_CONFIG related information */
	{
	    .image_id = TB_FW_CONFIG_ID,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_2, image_info_t, 0),
	    .image_info.image_base = TB_FW_CONFIG_BASE,
	    .image_info.image_max_size = TB_FW_CONFIG_LIMIT - TB_FW_CONFIG_BASE,
	    .next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/*
	 * Empty entries for SP packages to be filled in according to
	 * TB_FW_CONFIG.
	 */
	SP_PKG_ENTRY(SP_PKG1_ID),
	SP_PKG_ENTRY(SP_PKG2_ID),
	SP_PKG_ENTRY(SP_PKG3_ID),
	SP_PKG_ENTRY(SP_PKG4_ID),
	SP_PKG_ENTRY(SP_PKG5_ID),
	SP_PKG_ENTRY(SP_PKG6_ID),
	SP_PKG_ENTRY(SP_PKG7_ID),
	SP_PKG_ENTRY(SP_PKG8_ID),
#endif
#endif
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
	  .image_info.image_max_size = NS_IMAGE_MAX_SIZE,
# endif /* !PRELOADED_BL33_BASE */

	  .next_handoff_image_id = INVALID_IMAGE_ID,
	}
#endif /* !EL3_PAYLOAD_BASE */
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
