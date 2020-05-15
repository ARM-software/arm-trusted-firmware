/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FCONF_SDEI_GETTER_H
#define FCONF_SDEI_GETTER_H

#include <lib/fconf/fconf.h>

#include <platform_def.h>

#define sdei__dyn_config_getter(id)	sdei_dyn_config.id

struct sdei_dyn_config_t {
	uint32_t private_ev_cnt;
	int32_t private_ev_nums[PLAT_SDEI_DP_EVENT_MAX_CNT];
	unsigned int private_ev_intrs[PLAT_SDEI_DP_EVENT_MAX_CNT];
	unsigned int private_ev_flags[PLAT_SDEI_DP_EVENT_MAX_CNT];
	uint32_t shared_ev_cnt;
	int32_t shared_ev_nums[PLAT_SDEI_DS_EVENT_MAX_CNT];
	unsigned int shared_ev_intrs[PLAT_SDEI_DS_EVENT_MAX_CNT];
	unsigned int shared_ev_flags[PLAT_SDEI_DS_EVENT_MAX_CNT];
};

int fconf_populate_sdei_dyn_config(uintptr_t config);

extern struct sdei_dyn_config_t sdei_dyn_config;

#endif /* FCONF_SDEI_GETTER_H */
