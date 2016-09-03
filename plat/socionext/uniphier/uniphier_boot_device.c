/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <mmio.h>
#include <stdbool.h>
#include <stddef.h>
#include <utils_def.h>

#include "uniphier.h"

#define UNIPHIER_PINMON0		0x5f900100
#define UNIPHIER_PINMON2		0x5f900108

static int uniphier_ld11_is_usb_boot(uint32_t pinmon)
{
	return !!(~pinmon & 0x00000080);
}

static int uniphier_ld20_is_usb_boot(uint32_t pinmon)
{
	return !!(~pinmon & 0x00000780);
}

static int uniphier_pxs3_is_usb_boot(uint32_t pinmon)
{
	uint32_t pinmon2 = mmio_read_32(UNIPHIER_PINMON2);

	return !!(pinmon2 & BIT(31));
}

static const unsigned int uniphier_ld11_boot_device_table[] = {
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_NOR,
};

static unsigned int uniphier_ld11_get_boot_device(uint32_t pinmon)
{
	unsigned int boot_sel = (pinmon >> 1) & 0x1f;

	assert(boot_sel < ARRAY_SIZE(uniphier_ld11_boot_device_table));

	return uniphier_ld11_boot_device_table[boot_sel];
}

static const unsigned int uniphier_pxs3_boot_device_table[] = {
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
};

static unsigned int uniphier_pxs3_get_boot_device(uint32_t pinmon)
{
	unsigned int boot_sel = (pinmon >> 1) & 0xf;

	assert(boot_sel < ARRAY_SIZE(uniphier_pxs3_boot_device_table));

	return uniphier_pxs3_boot_device_table[boot_sel];
}

struct uniphier_boot_device_info {
	int (*is_usb_boot)(uint32_t pinmon);
	unsigned int (*get_boot_device)(uint32_t pinmon);
};

static const struct uniphier_boot_device_info uniphier_boot_device_info[] = {
	[UNIPHIER_SOC_LD11] = {
		.is_usb_boot = uniphier_ld11_is_usb_boot,
		.get_boot_device = uniphier_ld11_get_boot_device,
	},
	[UNIPHIER_SOC_LD20] = {
		.is_usb_boot = uniphier_ld20_is_usb_boot,
		.get_boot_device = uniphier_ld11_get_boot_device,
	},
	[UNIPHIER_SOC_PXS3] = {
		.is_usb_boot = uniphier_pxs3_is_usb_boot,
		.get_boot_device = uniphier_pxs3_get_boot_device,
	},
};

unsigned int uniphier_get_boot_device(unsigned int soc)
{
	const struct uniphier_boot_device_info *info;
	uint32_t pinmon;

	assert(soc < ARRAY_SIZE(uniphier_boot_device_info));
	info = &uniphier_boot_device_info[soc];

	pinmon = mmio_read_32(UNIPHIER_PINMON0);

	if (!(pinmon & BIT(29)))
		return UNIPHIER_BOOT_DEVICE_NOR;

	if (info->is_usb_boot(pinmon))
		return UNIPHIER_BOOT_DEVICE_USB;

	return info->get_boot_device(pinmon);
}

static const bool uniphier_have_onchip_scp[] = {
	[UNIPHIER_SOC_LD11] = true,
	[UNIPHIER_SOC_LD20] = true,
	[UNIPHIER_SOC_PXS3] = false,
};

unsigned int uniphier_get_boot_master(unsigned int soc)
{
	assert(soc < ARRAY_SIZE(uniphier_have_onchip_scp));

	if (uniphier_have_onchip_scp[soc]) {
		if (mmio_read_32(UNIPHIER_PINMON0) & BIT(27))
			return UNIPHIER_BOOT_MASTER_THIS;
		else
			return UNIPHIER_BOOT_MASTER_SCP;
	} else {
		return UNIPHIER_BOOT_MASTER_EXT;
	}
}
