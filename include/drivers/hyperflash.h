/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_HYPERFLASH_H
#define DRIVERS_HYPERFLASH_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

struct hyperflash_ctrl_ops {
	int (*read)(unsigned int offset, uint8_t *buffer, size_t length,
		    size_t *length_read);
};

struct hyperflash_device {
	const struct hyperflash_ctrl_ops *ops;
	uint32_t size;
};

int hyperflash_read(unsigned int offset, uintptr_t buffer, size_t length,
		    size_t *length_read);
int hyperflash_init(unsigned long long *size, unsigned int *erase_size);
void hyperflash_ctrl_init(const struct hyperflash_ctrl_ops *ops);

/*
 * Platform can implement this to override default hyperflash
 * configuration.
 *
 * @device: target hyperflash device.
 * Return 0 on success, negative value otherwise.
 */
int plat_get_hyperflash_data(struct hyperflash_device *device);

#endif	/* DRIVERS_HYPERFLASH_H */
