/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>
#include <smccc_helpers.h>
#include <tools_share/firmware_encrypted.h>

/*
 * The following platform functions are weakly defined. The Platforms
 * may redefine with strong definition.
 */
#pragma weak bl2_el3_plat_prepare_exit
#pragma weak plat_error_handler
#pragma weak bl2_plat_preload_setup
#pragma weak bl2_plat_handle_pre_image_load
#pragma weak bl2_plat_handle_post_image_load
#pragma weak plat_try_next_boot_source
#pragma weak plat_get_enc_key_info
#pragma weak plat_is_smccc_feature_available
#pragma weak plat_get_soc_version
#pragma weak plat_get_soc_revision

int32_t plat_get_soc_version(void)
{
	return SMC_ARCH_CALL_NOT_SUPPORTED;
}

int32_t plat_get_soc_revision(void)
{
	return SMC_ARCH_CALL_NOT_SUPPORTED;
}

int32_t plat_is_smccc_feature_available(u_register_t fid __unused)
{
	return SMC_ARCH_CALL_NOT_SUPPORTED;
}

void bl2_el3_plat_prepare_exit(void)
{
}

void __dead2 plat_error_handler(int err)
{
	while (1)
		wfi();
}

void bl2_plat_preload_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	return 0;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

int plat_try_next_boot_source(void)
{
	return 0;
}

/*
 * Weak implementation to provide dummy decryption key only for test purposes,
 * platforms must override this API for any real world firmware encryption
 * use-case.
 */
int plat_get_enc_key_info(enum fw_enc_status_t fw_enc_status, uint8_t *key,
			  size_t *key_len, unsigned int *flags,
			  const uint8_t *img_id, size_t img_id_len)
{
#define DUMMY_FIP_ENC_KEY { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, \
			    0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, \
			    0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, \
			    0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef }

	const uint8_t dummy_key[] = DUMMY_FIP_ENC_KEY;

	assert(*key_len >= sizeof(dummy_key));

	*key_len = sizeof(dummy_key);
	memcpy(key, dummy_key, *key_len);
	*flags = 0;

	return 0;
}

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The size of the Trusted SRAM seen by the BL image must be specified as well
 * as an array specifying the generic memory regions which can be;
 * - Code section;
 * - Read-only data section;
 * - Init code section, if applicable
 * - Coherent memory region, if applicable.
 */

void __init setup_page_tables(const mmap_region_t *bl_regions,
			      const mmap_region_t *plat_regions)
{
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	const mmap_region_t *regions = bl_regions;

	while (regions->size != 0U) {
		VERBOSE("Region: 0x%lx - 0x%lx has attributes 0x%x\n",
				regions->base_va,
				regions->base_va + regions->size,
				regions->attr);
		regions++;
	}
#endif
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	mmap_add(bl_regions);

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_regions);

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}
