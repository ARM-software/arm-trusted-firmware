/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IO_BLOCK_H__
#define __IO_BLOCK_H__

#include <io_storage.h>

/* block devices ops */
typedef struct io_block_ops {
	size_t	(*read)(int lba, uintptr_t buf, size_t size);
	size_t	(*write)(int lba, const uintptr_t buf, size_t size);
} io_block_ops_t;

typedef struct io_block_dev_spec {
	io_block_spec_t	buffer;
	io_block_ops_t	ops;
	size_t		block_size;
} io_block_dev_spec_t;

struct io_dev_connector;

int register_io_dev_block(const struct io_dev_connector **dev_con);

#endif /* __IO_BLOCK_H__ */
