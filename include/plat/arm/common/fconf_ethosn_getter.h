/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_ETHOSN_GETTER_H
#define FCONF_ETHOSN_GETTER_H

#include <assert.h>
#include <stdbool.h>

#include <lib/fconf/fconf.h>

#define hw_config__ethosn_config_getter(prop) ethosn_config.prop
#define hw_config__ethosn_device_getter(dev_idx) __extension__ ({	\
	assert(dev_idx < ethosn_config.num_devices);			\
	&ethosn_config.devices[dev_idx];				\
})

#define ETHOSN_DEV_NUM_MAX U(2)
#define ETHOSN_DEV_CORE_NUM_MAX U(8)
#define ETHOSN_DEV_ASSET_ALLOCATOR_NUM_MAX U(16)

struct ethosn_allocator_t {
	uint32_t stream_id;
};

struct ethosn_main_allocator_t {
	struct ethosn_allocator_t firmware;
	struct ethosn_allocator_t working_data;
};

struct ethosn_asset_allocator_t {
	struct ethosn_allocator_t command_stream;
	struct ethosn_allocator_t weight_data;
	struct ethosn_allocator_t buffer_data;
	struct ethosn_allocator_t intermediate_data;
};

struct ethosn_core_t {
	uint64_t addr;
	struct ethosn_main_allocator_t main_allocator;
};

struct ethosn_device_t {
	bool has_reserved_memory;
	uint32_t num_cores;
	struct ethosn_core_t cores[ETHOSN_DEV_CORE_NUM_MAX];
	uint32_t num_allocators;
	struct ethosn_asset_allocator_t asset_allocators[ETHOSN_DEV_ASSET_ALLOCATOR_NUM_MAX];
};

struct ethosn_config_t {
	uint32_t num_devices;
	struct ethosn_device_t devices[ETHOSN_DEV_NUM_MAX];
};

int fconf_populate_arm_ethosn(uintptr_t config);

extern struct ethosn_config_t ethosn_config;

#endif /* FCONF_ETHOSN_GETTER_H */
