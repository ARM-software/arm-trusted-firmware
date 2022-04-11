/*
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>
#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>
#if CRYPTO_SUPPORT
#include MBEDTLS_CONFIG_FILE
#endif /* CRYPTO_SUPPORT */
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/fconf/fconf_tbbr_getter.h>

#include <plat/arm/common/arm_dyn_cfg_helpers.h>
#include <plat/arm/common/plat_arm.h>

#if CRYPTO_SUPPORT

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

	/* If in BL2, retrieve the already allocated heap's info from DTB */
	*heap_addr = FCONF_GET_PROPERTY(tbbr, dyn_config, mbedtls_heap_addr);
	*heap_size = FCONF_GET_PROPERTY(tbbr, dyn_config, mbedtls_heap_size);

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
	uintptr_t tb_fw_cfg_dtb;
	const struct dyn_cfg_dtb_info_t *tb_fw_config_info;

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

	tb_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, TB_FW_CONFIG_ID);
	assert(tb_fw_config_info != NULL);

	tb_fw_cfg_dtb = tb_fw_config_info->config_addr;

	if ((tb_fw_cfg_dtb != 0UL) && (mbedtls_heap_addr != NULL)) {
		/* As libfdt uses void *, we can't avoid this cast */
		void *dtb = (void *)tb_fw_cfg_dtb;

		err = arm_set_dtb_mbedtls_heap_info(dtb,
			mbedtls_heap_addr, mbedtls_heap_size);
		if (err < 0) {
			ERROR("%swrite shared Mbed TLS heap information%s",
				"BL1: unable to ", " to DTB\n");
			panic();
		}
#if !MEASURED_BOOT
		/*
		 * Ensure that the info written to the DTB is visible to other
		 * images. It's critical because BL2 won't be able to proceed
		 * without the heap info.
		 *
		 * In MEASURED_BOOT case flushing is done in a function which
		 * is called after heap information is written in the DTB.
		 */
		flush_dcache_range(tb_fw_cfg_dtb, fdt_totalsize(dtb));
#endif /* !MEASURED_BOOT */
	}
}
#endif /* CRYPTO_SUPPORT */

/*
 * BL2 utility function to initialize dynamic configuration specified by
 * FW_CONFIG. Populate the bl_mem_params_node_t of other FW_CONFIGs if
 * specified in FW_CONFIG.
 */
void arm_bl2_dyn_cfg_init(void)
{
	unsigned int i;
	bl_mem_params_node_t *cfg_mem_params = NULL;
	uintptr_t image_base;
	uint32_t image_size;
	const unsigned int config_ids[] = {
			HW_CONFIG_ID,
			SOC_FW_CONFIG_ID,
			NT_FW_CONFIG_ID,
			TOS_FW_CONFIG_ID
	};

	const struct dyn_cfg_dtb_info_t *dtb_info;

	/* Iterate through all the fw config IDs */
	for (i = 0; i < ARRAY_SIZE(config_ids); i++) {
		/* Get the config load address and size from TB_FW_CONFIG */
		cfg_mem_params = get_bl_mem_params_node(config_ids[i]);
		if (cfg_mem_params == NULL) {
			VERBOSE("%sHW_CONFIG in bl_mem_params_node\n",
				"Couldn't find ");
			continue;
		}

		dtb_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, config_ids[i]);
		if (dtb_info == NULL) {
			VERBOSE("%sconfig_id %d load info in TB_FW_CONFIG\n",
				"Couldn't find ", config_ids[i]);
			continue;
		}

		image_base = dtb_info->config_addr;
		image_size = dtb_info->config_max_size;

		/*
		 * Do some runtime checks on the load addresses of soc_fw_config,
		 * tos_fw_config, nt_fw_config. This is not a comprehensive check
		 * of all invalid addresses but to prevent trivial porting errors.
		 */
		if (config_ids[i] != HW_CONFIG_ID) {

			if (check_uptr_overflow(image_base, image_size)) {
				continue;
			}
#ifdef	BL31_BASE
			/* Ensure the configs don't overlap with BL31 */
			if ((image_base >= BL31_BASE) &&
			    (image_base <= BL31_LIMIT)) {
				continue;
			}
#endif
			/* Ensure the configs are loaded in a valid address */
			if (image_base < ARM_BL_RAM_BASE) {
				continue;
			}
#ifdef BL32_BASE
			/*
			 * If BL32 is present, ensure that the configs don't
			 * overlap with it.
			 */
			if ((image_base >= BL32_BASE) &&
			    (image_base <= BL32_LIMIT)) {
				continue;
			}
#endif
		}

		cfg_mem_params->image_info.image_base = image_base;
		cfg_mem_params->image_info.image_max_size = (uint32_t)image_size;

		/*
		 * Remove the IMAGE_ATTRIB_SKIP_LOADING attribute from
		 * HW_CONFIG or FW_CONFIG nodes
		 */
		cfg_mem_params->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	}
}
