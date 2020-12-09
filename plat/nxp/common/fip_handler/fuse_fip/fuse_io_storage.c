/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include <dcfg.h>
#include <drivers/delay_timer.h>
#include <fuse_prov.h>
#include <io_block.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <lib/utils.h>
#include <nxp_gpio.h>
#include <sfp.h>
#include <sfp_error_codes.h>
#include <tools_share/firmware_image_package.h>

#include "fuse_io.h"
#include <load_img.h>
#include <plat/common/platform.h>
#include "plat_common.h"
#include "platform_def.h"

extern uintptr_t backend_dev_handle;

static uint32_t fuse_fip;

static uintptr_t fuse_fip_dev_handle;

static io_block_spec_t fuse_fip_block_spec = {
	.offset = PLAT_FUSE_FIP_OFFSET,
	.length = PLAT_FUSE_FIP_MAX_SIZE
};

static const io_uuid_spec_t fuse_prov_uuid_spec = {
	.uuid = UUID_FUSE_PROV,
};

static const io_uuid_spec_t fuse_up_uuid_spec = {
	.uuid = UUID_FUSE_UP,
};

static int open_fuse_fip(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy fuse_policies[] = {
	[FUSE_FIP_IMAGE_ID - FUSE_FIP_IMAGE_ID] = {
		&backend_dev_handle,
		(uintptr_t)&fuse_fip_block_spec,
		NULL
	},
	[FUSE_PROV_IMAGE_ID - FUSE_FIP_IMAGE_ID] = {
		&fuse_fip_dev_handle,
		(uintptr_t)&fuse_prov_uuid_spec,
		open_fuse_fip
	},
	[FUSE_UP_IMAGE_ID - FUSE_FIP_IMAGE_ID] = {
		&fuse_fip_dev_handle,
		(uintptr_t)&fuse_up_uuid_spec,
		open_fuse_fip
	}
};

static int open_fuse_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fuse_fip_dev_handle, (uintptr_t)FUSE_FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fuse_fip_dev_handle,
				 spec,
				 &local_image_handle);
		if (result == 0) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

/* The image can be one of the DDR PHY images, which can be sleected via DDR
 * policies
 */
int plat_get_fuse_image_source(unsigned int image_id,
			       uintptr_t *dev_handle,
			       uintptr_t *image_spec,
			       int (*check)(const uintptr_t spec))
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < (FUSE_FIP_IMAGE_ID + ARRAY_SIZE(fuse_policies)));

	policy = &fuse_policies[image_id - FUSE_FIP_IMAGE_ID];

	if (image_id == FUSE_FIP_IMAGE_ID) {
		result = check(policy->image_spec);
	} else {
		result = policy->check(policy->image_spec);
	}

	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}
	return result;
}

int fuse_fip_setup(const io_dev_connector_t *fip_dev_con, unsigned int boot_dev)
{
	int io_result;
	size_t fuse_fip_offset = PLAT_FUSE_FIP_OFFSET;

	/* Open connections to fuse fip and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)&fuse_fip,
				&fuse_fip_dev_handle);

	assert(io_result == 0);

	switch (boot_dev) {
#if QSPI_BOOT
	case BOOT_DEVICE_QSPI:
		fuse_fip_offset += NXP_QSPI_FLASH_ADDR;
		break;
#endif
#if NOR_BOOT
	case BOOT_DEVICE_IFC_NOR:
		fuse_fip_offset += NXP_NOR_FLASH_ADDR;
		break;
#endif
#if FLEXSPI_NOR_BOOT
	case BOOT_DEVICE_FLEXSPI_NOR:
		fuse_fip_offset += NXP_FLEXSPI_FLASH_ADDR;
		break;
#endif
	default:
		break;
	}

	fuse_fip_block_spec.offset = fuse_fip_offset;

	return io_result;
}

int fip_fuse_provisioning(uintptr_t image_buf, uint32_t size)
{
	uint32_t bit_num;
	uint32_t *gpio_base_addr = NULL;
	struct fuse_hdr_t *fuse_hdr = NULL;
	uint8_t barker[] = {0x68U, 0x39U, 0x27U, 0x81U};
	int ret = -1;

	if (sfp_check_oem_wp() == 0) {
		ret = load_img(FUSE_PROV_IMAGE_ID, &image_buf, &size);
		if (ret != 0) {
			ERROR("Failed to load FUSE PRIV image\n");
			assert(ret == 0);
		}
		fuse_hdr = (struct fuse_hdr_t *)image_buf;

		/* Check barker code */
		if (memcmp(fuse_hdr->barker, barker, sizeof(barker)) != 0) {
			ERROR("FUSE Barker code mismatch.\n");
			error_handler(ERROR_FUSE_BARKER);
			return 1;
		}

		/* Check if GPIO pin to be set for POVDD */
		if (((fuse_hdr->flags >> FLAG_POVDD_SHIFT) & 0x1) != 0) {
			gpio_base_addr =
				select_gpio_n_bitnum(fuse_hdr->povdd_gpio,
						     &bit_num);
			/*
			 * Add delay so that Efuse gets the power
			 * when GPIO is enabled.
			 */
			ret = set_gpio_bit(gpio_base_addr, bit_num);
			mdelay(EFUSE_POWERUP_DELAY_mSec);
		} else {
			ret = (board_enable_povdd() == true) ? 0 : PLAT_ERROR_ENABLE_POVDD;
		}
		if (ret != 0) {
			ERROR("Error enabling board POVDD: %d\n", ret);
			ERROR("Only SFP mirror register will be set.\n");
		}

		provision_fuses(image_buf, ret == 0);

		 /* Check if GPIO pin to be reset for POVDD */
		if (((fuse_hdr->flags >> FLAG_POVDD_SHIFT) & 0x1) != 0) {
			if (gpio_base_addr == NULL) {
				gpio_base_addr =
					select_gpio_n_bitnum(
							fuse_hdr->povdd_gpio,
							&bit_num);
			}
			ret = clr_gpio_bit(gpio_base_addr, bit_num);
		} else {
			ret = board_disable_povdd() ? 0 : PLAT_ERROR_DISABLE_POVDD;
		}

		if (ret != 0) {
			ERROR("Error disabling board POVDD: %d\n", ret);
		}
	}
	return 0;
}
