/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NVMEM_H
#define NVMEM_H

#include <stddef.h>
#include <stdint.h>

struct nvmem_dev {
	const struct nvmem_ops *nvmem_ops;
	void *drv_data;
	int offset;
};

struct nvmem_cell {
	int offset;
	size_t len;
	struct nvmem_dev *dev;
};

struct nvmem_ops {
	int (*read_cell)(struct nvmem_cell *cell, uint8_t *data, size_t buf_len,
			 size_t *read_len);
	int (*write_cell)(struct nvmem_cell *cell, uint8_t *data, size_t len);
};

int nvmem_driver_register(int nodeoffset, const struct nvmem_ops *nvmem_ops,
			  void *drv_data);

int nvmem_get_cell_by_index(const void *fdt, int nodeoffset, unsigned int index,
			    struct nvmem_cell *cell);

int nvmem_get_cell_by_name(const void *fdt, int nodeoffset, const char *name,
			   struct nvmem_cell *cell);

int nvmem_cell_read(struct nvmem_cell *cell, uint8_t *output_buf,
		    size_t buf_len, size_t *read_len);

int nvmem_cell_write(struct nvmem_cell *cell, uint8_t *input_buf,
		     size_t buf_len);
#endif
