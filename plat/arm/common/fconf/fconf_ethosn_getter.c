/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <plat/arm/common/fconf_ethosn_getter.h>

struct ethosn_config_t ethosn_config = {0};

struct ethosn_sub_allocator_t {
	const char *name;
	size_t name_len;
	uint32_t stream_id;
};

static int fdt_node_read_reserved_memory_addr(const void *fdt,
					      int dev_node,
					      uint64_t *reserved_mem_addrs)
{
	uintptr_t addr;
	uint32_t phandle;
	int err;
	int mem_node;

	err = fdt_read_uint32(fdt, dev_node, "memory-region", &phandle);
	if (err != 0) {
		ERROR("FCONF: Failed to get reserved memory phandle\n");
		return err;
	}

	mem_node = fdt_node_offset_by_phandle(fdt, phandle);
	if (mem_node < 0) {
		ERROR("FCONF: Failed to find reserved memory node from phandle\n");
		return mem_node;
	}

	err = fdt_get_reg_props_by_index(fdt, mem_node, 0U, &addr, NULL);
	if (err != 0) {
		ERROR("FCONF: Failed to read reserved memory address\n");
		return err;
	}

	*reserved_mem_addrs = addr;

	return 0;
}

static bool fdt_node_has_reserved_memory(const void *fdt, int dev_node)
{
	return fdt_get_property(fdt, dev_node, "memory-region", NULL) != NULL;
}

static int fdt_node_get_iommus_stream_id(const void *fdt, int node, uint32_t *stream_id)
{
	int err;
	uint32_t iommus_array[2] = {0U};

	err = fdt_read_uint32_array(fdt, node, "iommus", 2U, iommus_array);
	if (err) {
		return err;
	}

	*stream_id = iommus_array[1];
	return 0;
}

static int fdt_node_populate_sub_allocators(const void *fdt,
					    int alloc_node,
					    struct ethosn_sub_allocator_t *sub_allocators,
					    size_t num_allocs)
{
	int sub_node;
	size_t i;
	int err = -FDT_ERR_NOTFOUND;
	uint32_t found_sub_allocators = 0U;

	fdt_for_each_subnode(sub_node, fdt, alloc_node) {
		const char *node_name;

		if (!fdt_node_is_enabled(fdt, sub_node)) {
			/* Ignore disabled node */
			continue;
		}

		if (fdt_node_check_compatible(fdt, sub_node, "ethosn-memory") != 0) {
			continue;
		}

		node_name = fdt_get_name(fdt, sub_node, NULL);
		for (i = 0U; i < num_allocs; ++i) {
			if (strncmp(node_name, sub_allocators[i].name,
				    sub_allocators[i].name_len) != 0) {
				continue;
			}

			err = fdt_node_get_iommus_stream_id(fdt, sub_node,
							    &sub_allocators[i].stream_id);
			if (err) {
				ERROR("FCONF: Failed to get stream ID from sub-allocator %s\n",
				      node_name);
				return err;
			}

			++found_sub_allocators;
			/* Nothing more to do for this node */
			break;
		}

		/* Check that at least one of the sub-allocators matched */
		if (i == num_allocs) {
			ERROR("FCONF: Unknown sub-allocator %s\n", node_name);
			return -FDT_ERR_BADSTRUCTURE;
		}
	}

	if ((sub_node < 0) && (sub_node != -FDT_ERR_NOTFOUND)) {
		ERROR("FCONF: Failed to parse sub-allocators\n");
		return -FDT_ERR_BADSTRUCTURE;
	}

	if (err == -FDT_ERR_NOTFOUND) {
		ERROR("FCONF: No matching sub-allocator found\n");
		return err;
	}

	if (found_sub_allocators != num_allocs) {
		ERROR("FCONF: Not all sub-allocators were found\n");
		return -FDT_ERR_BADSTRUCTURE;
	}

	return 0;
}

static int fdt_node_populate_main_allocator(const void *fdt,
					    int alloc_node,
					    struct ethosn_main_allocator_t *allocator)
{
	int err;
	struct ethosn_sub_allocator_t sub_allocators[] = {
		{.name = "firmware", .name_len = 8U},
		{.name = "working_data", .name_len = 12U}
	};

	err = fdt_node_populate_sub_allocators(fdt, alloc_node, sub_allocators,
					       ARRAY_SIZE(sub_allocators));
	if (err) {
		return err;
	}

	allocator->firmware.stream_id = sub_allocators[0].stream_id;
	allocator->working_data.stream_id = sub_allocators[1].stream_id;

	return 0;
}

static int fdt_node_populate_asset_allocator(const void *fdt,
					    int alloc_node,
					    struct ethosn_asset_allocator_t *allocator)
{
	int err;
	struct ethosn_sub_allocator_t sub_allocators[] = {
		{.name = "command_stream", .name_len = 14U},
		{.name = "weight_data", .name_len = 11U},
		{.name = "buffer_data", .name_len = 11U},
		{.name = "intermediate_data", .name_len = 17U}
	};

	err = fdt_node_populate_sub_allocators(fdt, alloc_node, sub_allocators,
					       ARRAY_SIZE(sub_allocators));
	if (err) {
		return err;
	}


	allocator->command_stream.stream_id = sub_allocators[0].stream_id;
	allocator->weight_data.stream_id = sub_allocators[1].stream_id;
	allocator->buffer_data.stream_id = sub_allocators[2].stream_id;
	allocator->intermediate_data.stream_id = sub_allocators[3].stream_id;
	return 0;
}

static int fdt_node_populate_core(const void *fdt,
				  int device_node,
				  int core_node,
				  bool has_reserved_memory,
				  uint32_t core_index,
				  struct ethosn_core_t *core)
{
	int err;
	int sub_node;
	uintptr_t core_addr;

	err = fdt_get_reg_props_by_index(fdt, device_node, core_index,
					 &core_addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property for NPU core %u\n",
		      core_index);
		return err;
	}

	err = -FDT_ERR_NOTFOUND;
	fdt_for_each_subnode(sub_node, fdt, core_node) {

		if (!fdt_node_is_enabled(fdt, sub_node)) {
			continue;
		}

		if (fdt_node_check_compatible(fdt,
					      sub_node,
					      "ethosn-main_allocator") != 0) {
			continue;
		}

		if (has_reserved_memory) {
			ERROR("FCONF: Main allocator not supported when using reserved memory\n");
			return -FDT_ERR_BADSTRUCTURE;
		}

		if (err != -FDT_ERR_NOTFOUND) {
			ERROR("FCONF: NPU core 0x%lx has more than one main allocator\n",
			      core_addr);
			return -FDT_ERR_BADSTRUCTURE;
		}

		err = fdt_node_populate_main_allocator(fdt, sub_node, &core->main_allocator);
		if (err) {
			ERROR("FCONF: Failed to parse main allocator for NPU core 0x%lx\n",
			      core_addr);
			return err;
		}
	}

	if ((sub_node < 0) && (sub_node != -FDT_ERR_NOTFOUND)) {
		ERROR("FCONF: Failed to parse core sub nodes\n");
		return -FDT_ERR_BADSTRUCTURE;
	}

	if (!has_reserved_memory && err) {
		ERROR("FCONF: Main allocator not found for NPU core 0x%lx\n",
		      core_addr);
		return err;
	}

	core->addr = core_addr;

	return 0;
}

int fconf_populate_ethosn_config(uintptr_t config)
{
	int ethosn_node;
	uint32_t dev_count = 0U;
	const void *hw_conf_dtb = (const void *)config;

	INFO("Probing Arm(R) Ethos(TM)-N NPU\n");

	fdt_for_each_compatible_node(hw_conf_dtb, ethosn_node, "ethosn") {
		struct ethosn_device_t *dev = &ethosn_config.devices[dev_count];
		uint32_t dev_asset_alloc_count = 0U;
		uint32_t dev_core_count = 0U;
		uint64_t reserved_memory_addr = 0U;
		bool has_reserved_memory;
		int sub_node;
		int err;

		if (!fdt_node_is_enabled(hw_conf_dtb, ethosn_node)) {
			continue;
		}

		if (dev_count >= ETHOSN_DEV_NUM_MAX) {
			ERROR("FCONF: Reached max number of NPUs\n");
			return -FDT_ERR_BADSTRUCTURE;
		}

		has_reserved_memory = fdt_node_has_reserved_memory(hw_conf_dtb, ethosn_node);
		if (has_reserved_memory) {
			err = fdt_node_read_reserved_memory_addr(hw_conf_dtb,
								 ethosn_node,
								 &reserved_memory_addr);
			if (err != 0) {
				return err;
			}
		}

		fdt_for_each_subnode(sub_node, hw_conf_dtb, ethosn_node) {

			if (!fdt_node_is_enabled(hw_conf_dtb, sub_node)) {
				/* Ignore disabled sub node */
				continue;
			}

			if (fdt_node_check_compatible(hw_conf_dtb,
						      sub_node,
						      "ethosn-core") == 0) {

				if (dev_core_count >= ETHOSN_DEV_CORE_NUM_MAX) {
					ERROR("FCONF: Reached max number of NPU cores for NPU %u\n",
					      dev_count);
					return -FDT_ERR_BADSTRUCTURE;
				}

				err = fdt_node_populate_core(hw_conf_dtb,
							     ethosn_node,
							     sub_node,
							     has_reserved_memory,
							     dev_core_count,
							     &(dev->cores[dev_core_count]));
				if (err) {
					return err;
				}
				++dev_core_count;
			} else if (fdt_node_check_compatible(hw_conf_dtb,
							     sub_node,
							     "ethosn-asset_allocator") == 0) {

				if (dev_asset_alloc_count >=
				    ETHOSN_DEV_ASSET_ALLOCATOR_NUM_MAX) {
					ERROR("FCONF: Reached max number of asset allocators for NPU %u\n",
					      dev_count);
					return -FDT_ERR_BADSTRUCTURE;
				}

				if (has_reserved_memory) {
					ERROR("FCONF: Asset allocator not supported when using reserved memory\n");
					return -FDT_ERR_BADSTRUCTURE;
				}

				err = fdt_node_populate_asset_allocator(hw_conf_dtb,
									sub_node,
									&(dev->asset_allocators[dev_asset_alloc_count]));
				if (err) {
					ERROR("FCONF: Failed to parse asset allocator for NPU %u\n",
					      dev_count);
					return err;
				}
				++dev_asset_alloc_count;
			}
		}

		if ((sub_node < 0) && (sub_node != -FDT_ERR_NOTFOUND)) {
			ERROR("FCONF: Failed to parse sub nodes for NPU %u\n",
			      dev_count);
			return -FDT_ERR_BADSTRUCTURE;
		}

		if (dev_core_count == 0U) {
			ERROR("FCONF: NPU %u must have at least one enabled core\n",
			      dev_count);
			return -FDT_ERR_BADSTRUCTURE;
		}

		if (!has_reserved_memory && dev_asset_alloc_count == 0U) {
			ERROR("FCONF: NPU %u must have at least one asset allocator\n",
			      dev_count);
			return -FDT_ERR_BADSTRUCTURE;
		}

		dev->num_cores = dev_core_count;
		dev->num_allocators = dev_asset_alloc_count;
		dev->has_reserved_memory = has_reserved_memory;
		dev->reserved_memory_addr = reserved_memory_addr;
		++dev_count;
	}

	if (dev_count == 0U) {
		ERROR("FCONF: Can't find 'ethosn' compatible node in dtb\n");
		return -FDT_ERR_BADSTRUCTURE;
	}

	ethosn_config.num_devices = dev_count;

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, ethosn_config, fconf_populate_ethosn_config);
