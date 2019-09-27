/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_storage.h>
#include <drivers/st/bsec.h>
#include <drivers/st/stm32_hash.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

static const struct stm32mp_auth_ops *auth_ops;

void stm32mp_init_auth(struct stm32mp_auth_ops *init_ptr)
{
	if ((init_ptr == NULL) ||
	    (init_ptr->check_key == NULL) ||
	    (init_ptr->verify_signature == NULL) ||
	    (stm32_hash_register() != 0)) {
		panic();
	}

	auth_ops = init_ptr;
}

int stm32mp_auth_image(boot_api_image_header_t *header, uintptr_t buffer)
{
	int ret;
	uint8_t image_hash[BOOT_API_SHA256_DIGEST_SIZE_IN_BYTES];
	uint32_t header_skip_cksum = sizeof(header->magic) +
				     sizeof(header->image_signature) +
				     sizeof(header->payload_checksum);

	/* Check Security Status */
	if (!stm32mp_is_closed_device()) {
		if (header->option_flags != 0U) {
			WARN("Skip signature check (header option)\n");
			return 0;
		}
		INFO("Check signature on Open device\n");
	}

	ret = mmap_add_dynamic_region(STM32MP_ROM_BASE, STM32MP_ROM_BASE,
				      STM32MP_ROM_SIZE, MT_CODE | MT_SECURE);
	if (ret != 0) {
		return ret;
	}

	/* Check Public Key */
	if (auth_ops->check_key(header->ecc_pubk, NULL) != BOOT_API_RETURN_OK) {
		ret = -EINVAL;
		goto err;
	}

	/* Compute end of header hash and payload hash */
	stm32_hash_init(HASH_SHA256);

	ret = stm32_hash_update((uint8_t *)&header->header_version,
				sizeof(boot_api_image_header_t) -
				header_skip_cksum);
	if (ret != 0) {
		ERROR("Hash of header failed, %i\n", ret);
		goto err;
	}

	ret = stm32_hash_final_update((uint8_t *)buffer,
			       header->image_length, image_hash);
	if (ret != 0) {
		ERROR("Hash of payload failed\n");
		goto err;
	}

	/* Verify signature */
	if (auth_ops->verify_signature(image_hash, header->ecc_pubk,
				       header->image_signature,
				       header->ecc_algo_type) !=
	    BOOT_API_RETURN_OK) {
		ret = -EINVAL;
	}

err:
	mmap_remove_dynamic_region(STM32MP_ROM_BASE, STM32MP_ROM_SIZE);
	return ret;
}
