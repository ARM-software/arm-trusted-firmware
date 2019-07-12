/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "rom_api.h"

typedef int32_t(*secure_boot_api_f) (uint32_t a, uint32_t b, void *c);
extern int32_t rcar_get_certificate(const int32_t name, uint32_t *cert_addr);

#define	RCAR_IMAGE_ID_MAX	(10)
#define	RCAR_CERT_MAGIC_NUM	(0xE291F358U)
#define RCAR_BOOT_KEY_CERT	(0xE6300C00U)
#define RCAR_BOOT_KEY_CERT_NEW	(0xE6300F00U)
#define	RST_BASE		(0xE6160000U)
#define	RST_MODEMR		(RST_BASE + 0x0060U)
#define	MFISOFTMDR		(0xE6260600U)
#define	MODEMR_MD5_MASK		(0x00000020U)
#define	MODEMR_MD5_SHIFT	(5U)
#define	SOFTMD_BOOTMODE_MASK	(0x00000001U)
#define	SOFTMD_NORMALBOOT	(0x1U)

static secure_boot_api_f secure_boot_api;

int auth_mod_get_parent_id(unsigned int img_id, unsigned int *parent_id)
{
	return 1;
}

int auth_mod_verify_img(unsigned int img_id, void *ptr, unsigned int len)
{
	int32_t ret = 0, index = 0;
	uint32_t cert_addr = 0U;
	static const struct img_to_cert_t {
		uint32_t id;
		int32_t cert;
		const char *name;
	} image[RCAR_IMAGE_ID_MAX] = {
		{ BL31_IMAGE_ID, SOC_FW_CONTENT_CERT_ID, "BL31" },
		{ BL32_IMAGE_ID, TRUSTED_OS_FW_CONTENT_CERT_ID, "BL32" },
		{ BL33_IMAGE_ID, NON_TRUSTED_FW_CONTENT_CERT_ID, "BL33" },
		{ BL332_IMAGE_ID, BL332_CERT_ID, "BL332" },
		{ BL333_IMAGE_ID, BL333_CERT_ID, "BL333" },
		{ BL334_IMAGE_ID, BL334_CERT_ID, "BL334" },
		{ BL335_IMAGE_ID, BL335_CERT_ID, "BL335" },
		{ BL336_IMAGE_ID, BL336_CERT_ID, "BL336" },
		{ BL337_IMAGE_ID, BL337_CERT_ID, "BL337" },
		{ BL338_IMAGE_ID, BL338_CERT_ID, "BL338" },
	};

#if IMAGE_BL2
	switch (img_id) {
	case TRUSTED_KEY_CERT_ID:
	case SOC_FW_KEY_CERT_ID:
	case TRUSTED_OS_FW_KEY_CERT_ID:
	case NON_TRUSTED_FW_KEY_CERT_ID:
	case BL332_KEY_CERT_ID:
	case BL333_KEY_CERT_ID:
	case BL334_KEY_CERT_ID:
	case BL335_KEY_CERT_ID:
	case BL336_KEY_CERT_ID:
	case BL337_KEY_CERT_ID:
	case BL338_KEY_CERT_ID:
	case SOC_FW_CONTENT_CERT_ID:
	case TRUSTED_OS_FW_CONTENT_CERT_ID:
	case NON_TRUSTED_FW_CONTENT_CERT_ID:
	case BL332_CERT_ID:
	case BL333_CERT_ID:
	case BL334_CERT_ID:
	case BL335_CERT_ID:
	case BL336_CERT_ID:
	case BL337_CERT_ID:
	case BL338_CERT_ID:
		return ret;
	case BL31_IMAGE_ID:
	case BL32_IMAGE_ID:
	case BL33_IMAGE_ID:
	case BL332_IMAGE_ID:
	case BL333_IMAGE_ID:
	case BL334_IMAGE_ID:
	case BL335_IMAGE_ID:
	case BL336_IMAGE_ID:
	case BL337_IMAGE_ID:
	case BL338_IMAGE_ID:
		goto verify_image;
	default:
		return -1;
	}

verify_image:
	for (index = 0; index < RCAR_IMAGE_ID_MAX; index++) {
		if (img_id != image[index].id)
			continue;

		ret = rcar_get_certificate(image[index].cert, &cert_addr);
		break;
	}

	if (ret || (index == RCAR_IMAGE_ID_MAX)) {
		ERROR("Verification Failed for image id = %d\n", img_id);
		return ret;
	}
#if RCAR_BL2_DCACHE == 1
	/* clean and disable */
	write_sctlr_el3(read_sctlr_el3() & ~SCTLR_C_BIT);
	dcsw_op_all(DCCISW);
#endif
	ret = (mmio_read_32(RCAR_BOOT_KEY_CERT_NEW) == RCAR_CERT_MAGIC_NUM) ?
	    secure_boot_api(RCAR_BOOT_KEY_CERT_NEW, cert_addr, NULL) :
	    secure_boot_api(RCAR_BOOT_KEY_CERT, cert_addr, NULL);
	if (ret)
		ERROR("Verification Failed 0x%x, %s\n", ret, image[index].name);

#if RCAR_BL2_DCACHE == 1
	/* enable */
	write_sctlr_el3(read_sctlr_el3() | SCTLR_C_BIT);
#endif

#endif
	return ret;
}

static int32_t normal_boot_verify(uint32_t a, uint32_t b, void *c)
{
	return 0;
}

void auth_mod_init(void)
{
#if RCAR_SECURE_BOOT
	uint32_t soft_md = mmio_read_32(MFISOFTMDR) & SOFTMD_BOOTMODE_MASK;
	uint32_t md = mmio_read_32(RST_MODEMR) & MODEMR_MD5_MASK;
	uint32_t lcs, ret;

	secure_boot_api = (secure_boot_api_f) &rcar_rom_secure_boot_api;

	ret = rcar_rom_get_lcs(&lcs);
	if (ret) {
		ERROR("BL2: Failed to get the LCS. (%d)\n", ret);
		panic();
	}

	switch (lcs) {
	case LCS_SE:
		if (soft_md == SOFTMD_NORMALBOOT)
			secure_boot_api = &normal_boot_verify;
		break;
	case LCS_SD:
		secure_boot_api = &normal_boot_verify;
		break;
	default:
		if (md >> MODEMR_MD5_SHIFT)
			secure_boot_api = &normal_boot_verify;
	}

	NOTICE("BL2: %s boot\n",
	       secure_boot_api == &normal_boot_verify ? "Normal" : "Secure");
#else
	NOTICE("BL2: Normal boot\n");
	secure_boot_api = &normal_boot_verify;
#endif
}
