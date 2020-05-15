/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <plat/arm/common/fconf_sdei_getter.h>

#define PRIVATE_EVENT_NUM(i) private_events[3 * (i)]
#define PRIVATE_EVENT_INTR(i) private_events[3 * (i) + 1]
#define PRIVATE_EVENT_FLAGS(i) private_events[3 * (i) + 2]

#define SHARED_EVENT_NUM(i) shared_events[3 * (i)]
#define SHARED_EVENT_INTR(i) shared_events[3 * (i) + 1]
#define SHARED_EVENT_FLAGS(i) shared_events[3 * (i) + 2]

struct sdei_dyn_config_t sdei_dyn_config;

int fconf_populate_sdei_dyn_config(uintptr_t config)
{
	uint32_t i;
	int node, err;
	uint32_t private_events[PLAT_SDEI_DP_EVENT_MAX_CNT * 3];
	uint32_t shared_events[PLAT_SDEI_DS_EVENT_MAX_CNT * 3];

	const void *dtb = (void *)config;

	/* Check that the node offset points to compatible property */
	node = fdt_node_offset_by_compatible(dtb, -1, "arm,sdei-1.0");
	if (node < 0) {
		ERROR("FCONF: Can't find 'arm,sdei-1.0' compatible node in dtb\n");
		return node;
	}

	/* Read number of private mappings */
	err = fdt_read_uint32(dtb, node, "private_event_count",
				&sdei_dyn_config.private_ev_cnt);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'private_event_count': %u\n",
				sdei_dyn_config.private_ev_cnt);
		return err;
	}

	/* Check if the value is in range */
	if (sdei_dyn_config.private_ev_cnt > PLAT_SDEI_DP_EVENT_MAX_CNT) {
		ERROR("FCONF: Invalid value for 'private_event_count': %u\n",
				sdei_dyn_config.private_ev_cnt);
		return -1;
	}

	/* Read private mappings */
	err = fdt_read_uint32_array(dtb, node, "private_events",
				sdei_dyn_config.private_ev_cnt * 3, private_events);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'private_events': %d\n", err);
		return err;
	}

	/* Move data to fconf struct */
	for (i = 0; i < sdei_dyn_config.private_ev_cnt; i++) {
		sdei_dyn_config.private_ev_nums[i]  = PRIVATE_EVENT_NUM(i);
		sdei_dyn_config.private_ev_intrs[i] = PRIVATE_EVENT_INTR(i);
		sdei_dyn_config.private_ev_flags[i] = PRIVATE_EVENT_FLAGS(i);
	}

	/* Read number of shared mappings */
	err = fdt_read_uint32(dtb, node, "shared_event_count",
				&sdei_dyn_config.shared_ev_cnt);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'shared_event_count'\n");
		return err;
	}

	/* Check if the value is in range */
	if (sdei_dyn_config.shared_ev_cnt > PLAT_SDEI_DS_EVENT_MAX_CNT) {
		ERROR("FCONF: Invalid value for 'shared_event_count': %u\n",
				sdei_dyn_config.shared_ev_cnt);
		return -1;
	}

	/* Read shared mappings */
	err = fdt_read_uint32_array(dtb, node, "shared_events",
				sdei_dyn_config.shared_ev_cnt * 3, shared_events);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'shared_events': %d\n", err);
		return err;
	}

	/* Move data to fconf struct */
	for (i = 0; i < sdei_dyn_config.shared_ev_cnt; i++) {
		sdei_dyn_config.shared_ev_nums[i]  = SHARED_EVENT_NUM(i);
		sdei_dyn_config.shared_ev_intrs[i] = SHARED_EVENT_INTR(i);
		sdei_dyn_config.shared_ev_flags[i] = SHARED_EVENT_FLAGS(i);
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, sdei, fconf_populate_sdei_dyn_config);
