/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <io/io_block.h>
#include <mmio.h>
#include <platform_def.h>
#include <sys/types.h>
#include <utils_def.h>

#include "uniphier.h"

#define UNIPHIER_LD11_USB_DESC_BASE	0x30010000
#define UNIPHIER_LD20_USB_DESC_BASE	0x30014000
#define UNIPHIER_PXS3_USB_DESC_BASE	0x30014000

#define UNIPHIER_SRB_OCM_CONT		0x61200000

struct uniphier_ld11_trans_op {
	uint8_t __pad[48];
};

struct uniphier_ld11_op {
	uint8_t __pad[56];
	struct uniphier_ld11_trans_op *trans_op;
	void *__pad2;
	void *dev_desc;
};

struct uniphier_ld20_trans_op {
	uint8_t __pad[40];
};

struct uniphier_ld20_op {
	uint8_t __pad[192];
	struct uniphier_ld20_trans_op *trans_op;
	void *__pad2;
	void *dev_desc;
};

struct uniphier_pxs3_op {
	uint8_t __pad[184];
	struct uniphier_ld20_trans_op *trans_op;
	void *__pad2;
	void *dev_desc;
};

static int (*__uniphier_usb_read)(int lba, uintptr_t buf, size_t size);

static void uniphier_ld11_usb_init(void)
{
	struct uniphier_ld11_op *op = (void *)UNIPHIER_LD11_USB_DESC_BASE;

	op->trans_op = (void *)(op + 1);

	op->dev_desc = op->trans_op + 1;
}

static int uniphier_ld11_usb_read(int lba, uintptr_t buf, size_t size)
{
	static int (*rom_usb_read)(uintptr_t desc, unsigned int lba,
				   unsigned int size, uintptr_t buf);
	uintptr_t func_addr;

	func_addr = uniphier_get_soc_revision() == 1 ? 0x3880 : 0x3958;
	rom_usb_read = (__typeof(rom_usb_read))func_addr;

	return rom_usb_read(UNIPHIER_LD11_USB_DESC_BASE, lba, size, buf);
}

static void uniphier_ld20_usb_init(void)
{
	struct uniphier_ld20_op *op = (void *)UNIPHIER_LD20_USB_DESC_BASE;

	op->trans_op = (void *)(op + 1);

	op->dev_desc = op->trans_op + 1;
}

static int uniphier_ld20_usb_read(int lba, uintptr_t buf, size_t size)
{
	static int (*rom_usb_read)(uintptr_t desc, unsigned int lba,
				   unsigned int size, uintptr_t buf);
	int ret;

	rom_usb_read = (__typeof(rom_usb_read))0x37f0;

	mmio_write_32(UNIPHIER_SRB_OCM_CONT, 0x1ff);

	/* ROM-API - return 1 on success, 0 on error */
	ret = rom_usb_read(UNIPHIER_LD20_USB_DESC_BASE, lba, size, buf);

	mmio_write_32(UNIPHIER_SRB_OCM_CONT, 0);

	return ret ? 0 : -1;
}

static void uniphier_pxs3_usb_init(void)
{
	struct uniphier_pxs3_op *op = (void *)UNIPHIER_PXS3_USB_DESC_BASE;

	op->trans_op = (void *)(op + 1);

	op->dev_desc = op->trans_op + 1;
}

static int uniphier_pxs3_usb_read(int lba, uintptr_t buf, size_t size)
{
	static int (*rom_usb_read)(uintptr_t desc, unsigned int lba,
				   unsigned int size, uintptr_t buf);
	int ret;

	rom_usb_read = (__typeof(rom_usb_read))0x39e8;

	/* ROM-API - return 1 on success, 0 on error */
	ret = rom_usb_read(UNIPHIER_PXS3_USB_DESC_BASE, lba, size, buf);

	return ret ? 0 : -1;
}

struct uniphier_usb_rom_param {
	void (*init)(void);
	int (*read)(int lba, uintptr_t buf, size_t size);
};

static const struct uniphier_usb_rom_param uniphier_usb_rom_params[] = {
	[UNIPHIER_SOC_LD11] = {
		.init = uniphier_ld11_usb_init,
		.read = uniphier_ld11_usb_read,
	},
	[UNIPHIER_SOC_LD20] = {
		.init = uniphier_ld20_usb_init,
		.read = uniphier_ld20_usb_read,
	},
	[UNIPHIER_SOC_PXS3] = {
		.init = uniphier_pxs3_usb_init,
		.read = uniphier_pxs3_usb_read,
	},
};

static size_t uniphier_usb_read(int lba, uintptr_t buf, size_t size)
{
	int ret;

	inv_dcache_range(buf, size);

	ret = __uniphier_usb_read(lba, buf, size);

	inv_dcache_range(buf, size);

	return ret ? 0 : size;
}

static struct io_block_dev_spec uniphier_usb_dev_spec = {
	.buffer = {
		.offset = UNIPHIER_BLOCK_BUF_BASE,
		.length = UNIPHIER_BLOCK_BUF_SIZE,
	},
	.ops = {
		.read = uniphier_usb_read,
	},
	.block_size = 512,
};

int uniphier_usb_init(unsigned int soc, uintptr_t *block_dev_spec)
{
	const struct uniphier_usb_rom_param *param;

	assert(soc < ARRAY_SIZE(uniphier_usb_rom_params));
	param = &uniphier_usb_rom_params[soc];

	if (param->init)
		param->init();

	__uniphier_usb_read = param->read;

	*block_dev_spec = (uintptr_t)&uniphier_usb_dev_spec;

	return 0;
}
