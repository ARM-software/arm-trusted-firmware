/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <libfdt.h>
#include <libfdt_env.h>

#include <drivers/st/nvmem.h>

#include <platform_def.h>

#define for_each_nvmem_dev(nvmem_dev) \
	for ((nvmem_dev) = nvmem_dev_array; \
	     (nvmem_dev) <= &nvmem_dev_array[PLAT_NB_NVMEM_DEVS - 1U]; \
	     (nvmem_dev)++)

static struct nvmem_dev nvmem_dev_array[PLAT_NB_NVMEM_DEVS];

int nvmem_driver_register(int nodeoffset, const struct nvmem_ops *nvmem_ops,
			  void *drv_data)
{
	struct nvmem_dev *nvmem_dev = NULL;

	for_each_nvmem_dev(nvmem_dev) {
		if (nvmem_dev->drv_data == NULL) {
			break;
		}
	}

	if (nvmem_dev > &nvmem_dev_array[PLAT_NB_NVMEM_DEVS - 1U]) {
		WARN("Not enough place for NVMEM dev, PLAT_NB_NVMEM_DEVS should be increased.\n");
		return -ENOMEM;
	}

	nvmem_dev->nvmem_ops = nvmem_ops;
	nvmem_dev->drv_data = drv_data;
	nvmem_dev->offset = nodeoffset;

	return 0;
}

static struct nvmem_dev *nvmem_get_by_node(int node_offset)
{
	struct nvmem_dev *nvmem_dev = NULL;

	for_each_nvmem_dev(nvmem_dev) {
		if (nvmem_dev->offset == node_offset) {
			return nvmem_dev;
		}
	}

	WARN("%s: node %d not found\n", __func__, node_offset);
	return NULL;
}

int nvmem_get_cell_by_index(const void *fdt, int nodeoffset, unsigned int index,
			    struct nvmem_cell *cell)
{
	int len = 0;
	const fdt32_t *nvmem_cells_prop = NULL, *nvmem_cell_reg_prop = NULL;
	uint32_t phandle;
	int nvmem_cell_node;
	int parent_node;
	struct nvmem_dev *dev = NULL;

	nvmem_cells_prop = fdt_getprop(fdt, nodeoffset, "nvmem-cells", &len);
	if (nvmem_cells_prop == NULL) {
		ERROR("Can't find nvmem cell");
		return len;
	}

	phandle = fdt32_to_cpu(nvmem_cells_prop[index]);
	nvmem_cell_node = fdt_node_offset_by_phandle(fdt, phandle);
	if (nvmem_cell_node < 0) {
		return nvmem_cell_node;
	}

	nvmem_cell_reg_prop = fdt_getprop(fdt, nvmem_cell_node, "reg", &len);

	if ((nvmem_cell_reg_prop == NULL) ||
	    ((unsigned int)len != (sizeof(uint32_t) * U(2)))) {
		return -FDT_ERR_NOTFOUND;
	}

	cell->offset = fdt32_to_cpu(nvmem_cell_reg_prop[0]);
	cell->len = fdt32_to_cpu(nvmem_cell_reg_prop[1]);

	parent_node = fdt_parent_offset(fdt, nvmem_cell_node);
	dev = nvmem_get_by_node(parent_node);
	if (dev == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	cell->dev = dev;
	return 0;
}

int nvmem_get_cell_by_name(const void *fdt, int nodeoffset, const char *name,
			   struct nvmem_cell *cell)
{
	int index = fdt_stringlist_search(fdt, nodeoffset, "nvmem-cell-names", name);

	if (index < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return nvmem_get_cell_by_index(fdt, nodeoffset, (unsigned int)index,
				       cell);
}

int nvmem_cell_read(struct nvmem_cell *cell, uint8_t *output_buf,
		    size_t buf_len, size_t *read_len)
{
	if (cell->dev->nvmem_ops->read_cell == NULL) {
		return -ENOTSUP;
	}

	return cell->dev->nvmem_ops->read_cell(cell, output_buf, buf_len,
					       read_len);
}

int nvmem_cell_write(struct nvmem_cell *cell, uint8_t *input_buf,
		     size_t buf_len)
{
	if (cell->dev->nvmem_ops->write_cell == NULL) {
		return -ENOTSUP;
	}

	return cell->dev->nvmem_ops->write_cell(cell, input_buf, buf_len);
}
