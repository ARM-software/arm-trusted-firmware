/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_ETHOSN_GETTER_H
#define FCONF_ETHOSN_GETTER_H

#include <assert.h>

#include <lib/fconf/fconf.h>

#define hw_config__ethosn_config_getter(prop) ethosn_config.prop
#define hw_config__ethosn_core_addr_getter(idx) __extension__ ({	\
	assert(idx < ethosn_config.num_cores);				\
	ethosn_config.core[idx].addr;					\
})

#define ETHOSN_STATUS_DISABLED U(0)
#define ETHOSN_STATUS_ENABLED  U(1)

#define ETHOSN_CORE_NUM_MAX U(64)

struct ethosn_core_t {
	uint64_t addr;
};

struct ethosn_config_t {
	uint32_t num_cores;
	struct ethosn_core_t core[ETHOSN_CORE_NUM_MAX];
};

int fconf_populate_arm_ethosn(uintptr_t config);

extern struct ethosn_config_t ethosn_config;

#endif /* FCONF_ETHOSN_GETTER_H */
