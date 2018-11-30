/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>
#if TRUSTED_BOARD_BOOT
#include <drivers/auth/mbedtls/mbedtls_config.h>
#endif
#include <plat/arm/common/arm_dyn_cfg_helpers.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Variable to store the address to TB_FW_CONFIG passed from BL1 */
static void *tb_fw_cfg_dtb;
static size_t tb_fw_cfg_dtb_size;


#if TRUSTED_BOARD_BOOT

static void *mbedtls_heap_addr;
static size_t mbedtls_heap_size;

/*
 * This function is the implementation of the shared Mbed TLS heap between
 * BL1 and BL2 for Arm platforms. The shared heap address is passed from BL1
 * to BL2 with a pointer. This pointer resides inside the TB_FW_CONFIG file
 * which is a DTB.
 *
 * This function is placed inside an #if directive for the below reasons:
 *   - To allocate space for the Mbed TLS heap --only if-- Trusted Board Boot
 *     is enabled.
 *   - This implementation requires the DTB to be present so that BL1 has a
 *     mechanism to pass the pointer to BL2.
 */
int arm_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

#if defined(IMAGE_BL1) || BL2_AT_EL3

	/* If in BL1 or BL2_AT_EL3 define a heap */
	static unsigned char heap[TF_MBEDTLS_HEAP_SIZE];

	*heap_addr = heap;
	*heap_size = sizeof(heap);
	mbedtls_heap_addr = heap;
	mbedtls_heap_size = sizeof(heap);

#elif defined(IMAGE_BL2)

	int err;

	/* If in BL2, retrieve the already allocated heap's info from DTB */
	if (tb_fw_cfg_dtb != NULL) {
		err = arm_get_dtb_mbedtls_heap_info(tb_fw_cfg_dtb, heap_addr,
			heap_size);
		if (err < 0) {
			ERROR("BL2: unable to retrieve shared Mbed TLS heap information from DTB\n");
			panic();
		}
	} else {
		ERROR("BL2: DTB missing, cannot get Mbed TLS heap\n");
		panic();
	}
#endif

	return 0;
}

/*
 * Puts the shared Mbed TLS heap information to the DTB.
 * Executed only from BL1.
 */
void arm_bl1_set_mbedtls_heap(void)
{
	int err;

	/*
	 * If tb_fw_cfg_dtb==NULL then DTB is not present for the current
	 * platform. As such, we don't attempt to write to the DTB at all.
	 *
	 * If mbedtls_heap_addr==NULL, then it means we are using the default
	 * heap implementation. As such, BL2 will have its own heap for sure
	 * and hence there is no need to pass any information to the DTB.
	 *
	 * In the latter case, if we still wanted to write in the DTB the heap
	 * information, we would need to call plat_get_mbedtls_heap to retrieve
	 * the default heap's address and size.
	 */
	if ((tb_fw_cfg_dtb != NULL) && (mbedtls_heap_addr != NULL)) {
		err = arm_set_dtb_mbedtls_heap_info(tb_fw_cfg_dtb,
			mbedtls_heap_addr, mbedtls_heap_size);
		if (err < 0) {
			ERROR("BL1: unable to write shared Mbed TLS heap information to DTB\n");
			panic();
		}
		/*
		 * Ensure that the info written to the DTB is visible to other
		 * images. It's critical because BL2 won't be able to proceed
		 * without the heap info.
		 */
		flush_dcache_range((uintptr_t)tb_fw_cfg_dtb,
			tb_fw_cfg_dtb_size);
	}
}

#endif /* TRUSTED_BOARD_BOOT */

/*
 * Helper function to load TB_FW_CONFIG and populate the load information to
 * arg0 of BL2 entrypoint info.
 */
void arm_load_tb_fw_config(void)
{
	int err;
	uintptr_t config_base = 0UL;
	image_desc_t *desc;

	image_desc_t arm_tb_fw_info = {
		.image_id = TB_FW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				VERSION_2, image_info_t, 0),
		.image_info.image_base = ARM_TB_FW_CONFIG_BASE,
		.image_info.image_max_size =
			ARM_TB_FW_CONFIG_LIMIT - ARM_TB_FW_CONFIG_BASE
	};

	VERBOSE("BL1: Loading TB_FW_CONFIG\n");
	err = load_auth_image(TB_FW_CONFIG_ID, &arm_tb_fw_info.image_info);
	if (err != 0) {
		/* Return if TB_FW_CONFIG is not loaded */
		VERBOSE("Failed to load TB_FW_CONFIG\n");
		return;
	}

	/* At this point we know that a DTB is indeed available */
	config_base = arm_tb_fw_info.image_info.image_base;
	tb_fw_cfg_dtb = (void *)config_base;
	tb_fw_cfg_dtb_size = (size_t)arm_tb_fw_info.image_info.image_max_size;

	/* The BL2 ep_info arg0 is modified to point to TB_FW_CONFIG */
	desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(desc != NULL);
	desc->ep_info.args.arg0 = config_base;

	INFO("BL1: TB_FW_CONFIG loaded at address = 0x%lx\n", config_base);

#if TRUSTED_BOARD_BOOT && defined(DYN_DISABLE_AUTH)
	int tb_fw_node;
	uint32_t disable_auth = 0;

	err = arm_dyn_tb_fw_cfg_init((void *)config_base, &tb_fw_node);
	if (err < 0) {
		ERROR("Invalid TB_FW_CONFIG loaded\n");
		panic();
	}

	err = arm_dyn_get_disable_auth((void *)config_base, tb_fw_node, &disable_auth);
	if (err < 0)
		return;

	if (disable_auth == 1)
		dyn_disable_auth();
#endif
}

/*
 * BL2 utility function to set the address of TB_FW_CONFIG passed from BL1.
 */
void arm_bl2_set_tb_cfg_addr(void *dtb)
{
	assert(dtb != NULL);
	tb_fw_cfg_dtb = dtb;
}

/*
 * BL2 utility function to initialize dynamic configuration specified by
 * TB_FW_CONFIG. Populate the bl_mem_params_node_t of other FW_CONFIGs if
 * specified in TB_FW_CONFIG.
 */
void arm_bl2_dyn_cfg_init(void)
{
	int err = 0, tb_fw_node;
	unsigned int i;
	bl_mem_params_node_t *cfg_mem_params = NULL;
	uint64_t image_base;
	uint32_t image_size;
	const unsigned int config_ids[] = {
			HW_CONFIG_ID,
			SOC_FW_CONFIG_ID,
			NT_FW_CONFIG_ID,
#ifdef SPD_tspd
			/* Currently tos_fw_config is only present for TSP */
			TOS_FW_CONFIG_ID
#endif
	};

	if (tb_fw_cfg_dtb == NULL) {
		VERBOSE("No TB_FW_CONFIG specified\n");
		return;
	}

	err = arm_dyn_tb_fw_cfg_init(tb_fw_cfg_dtb, &tb_fw_node);
	if (err < 0) {
		ERROR("Invalid TB_FW_CONFIG passed from BL1\n");
		panic();
	}

	/* Iterate through all the fw config IDs */
	for (i = 0; i < ARRAY_SIZE(config_ids); i++) {
		/* Get the config load address and size from TB_FW_CONFIG */
		cfg_mem_params = get_bl_mem_params_node(config_ids[i]);
		if (cfg_mem_params == NULL) {
			VERBOSE("Couldn't find HW_CONFIG in bl_mem_params_node\n");
			continue;
		}

		err = arm_dyn_get_config_load_info(tb_fw_cfg_dtb, tb_fw_node,
				config_ids[i], &image_base, &image_size);
		if (err < 0) {
			VERBOSE("Couldn't find config_id %d load info in TB_FW_CONFIG\n",
					config_ids[i]);
			continue;
		}

		/*
		 * Do some runtime checks on the load addresses of soc_fw_config,
		 * tos_fw_config, nt_fw_config. This is not a comprehensive check
		 * of all invalid addresses but to prevent trivial porting errors.
		 */
		if (config_ids[i] != HW_CONFIG_ID) {

			if (check_uptr_overflow(image_base, image_size))
				continue;

#ifdef	BL31_BASE
			/* Ensure the configs don't overlap with BL31 */
			if ((image_base > BL31_BASE) || ((image_base + image_size) > BL31_BASE))
				continue;
#endif
			/* Ensure the configs are loaded in a valid address */
			if (image_base < ARM_BL_RAM_BASE)
				continue;
#ifdef BL32_BASE
			/*
			 * If BL32 is present, ensure that the configs don't
			 * overlap with it.
			 */
			if (image_base >= BL32_BASE && image_base <= BL32_LIMIT)
				continue;
#endif
		}


		cfg_mem_params->image_info.image_base = (uintptr_t)image_base;
		cfg_mem_params->image_info.image_max_size = image_size;

		/* Remove the IMAGE_ATTRIB_SKIP_LOADING attribute from HW_CONFIG node */
		cfg_mem_params->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	}

#if TRUSTED_BOARD_BOOT && defined(DYN_DISABLE_AUTH)
	uint32_t disable_auth = 0;

	err = arm_dyn_get_disable_auth(tb_fw_cfg_dtb, tb_fw_node,
					&disable_auth);
	if (err < 0)
		return;

	if (disable_auth == 1)
		dyn_disable_auth();
#endif
}
