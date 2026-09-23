/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/hyperflash.h>
#include <lib/utils.h>

static struct hyperflash_device hyperflash_dev;

#pragma weak plat_get_hyperflash_data
int plat_get_hyperflash_data(struct hyperflash_device *device)
{
	return 0;
}

int hyperflash_read(unsigned int offset, uintptr_t buffer, size_t length,
		    size_t *length_read)
{
	return hyperflash_dev.ops->read(offset, (uint8_t *)buffer, length,
					length_read);
}

void hyperflash_ctrl_init(const struct hyperflash_ctrl_ops *ops)
{
	hyperflash_dev.ops = ops;
}

int hyperflash_init(unsigned long long *size, unsigned int *erase_size)
{
	if (hyperflash_dev.ops->read == NULL) {
		return -ENODEV;
	}

	if (plat_get_hyperflash_data(&hyperflash_dev) != 0) {
		return -EINVAL;
	}

	assert(hyperflash_dev.size != 0U);

	*size = hyperflash_dev.size;

	return 0;
}
