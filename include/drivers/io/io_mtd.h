/*
 * Copyright (c) 2019-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_MTD_H
#define IO_MTD_H

#include <stdint.h>
#include <stdio.h>

#include <drivers/io/io_storage.h>

/* MTD devices ops */
typedef struct io_mtd_ops {
	/*
	 * Initialize MTD framework and retrieve device information.
	 *
	 * @size:  [out] MTD device size in bytes.
	 * @erase_size: [out] MTD erase size in bytes.
	 * Return 0 on success, a negative error code otherwise.
	 */
	int (*init)(unsigned long long *size, unsigned int *erase_size);

	/*
	 * Execute a read memory operation.
	 *
	 * @offset: Offset in bytes to start read operation.
	 * @buffer: [out] Buffer to store read data.
	 * @length: Required length to be read in bytes.
	 * @out_length: [out] Length read in bytes.
	 * Return 0 on success, a negative error code otherwise.
	 */
	int (*read)(unsigned int offset, uintptr_t buffer, size_t length,
		    size_t *out_length);

	/*
	 * Execute a write memory operation.
	 *
	 * @offset: Offset in bytes to start write operation.
	 * @buffer: Buffer to be written in device.
	 * @length: Required length to be written in bytes.
	 * Return 0 on success, a negative error code otherwise.
	 */
	int (*write)(unsigned int offset, uintptr_t buffer, size_t length);

	/*
	 * Look for an offset to be added to the given offset.
	 *
	 * @base: Base address of the area.
	 * @offset: Offset in bytes to start read operation.
	 * @extra_offset: [out] Offset to be added to the previous offset.
	 * Return 0 on success, a negative error code otherwise.
	 */
	int (*seek)(uintptr_t base, unsigned int offset, size_t *extra_offset);
} io_mtd_ops_t;

typedef struct io_mtd_dev_spec {
	unsigned long long device_size;
	unsigned int erase_size;
	size_t offset;
	io_mtd_ops_t ops;
} io_mtd_dev_spec_t;

struct io_dev_connector;

int register_io_dev_mtd(const struct io_dev_connector **dev_con);

#endif /* IO_MTD_H */
