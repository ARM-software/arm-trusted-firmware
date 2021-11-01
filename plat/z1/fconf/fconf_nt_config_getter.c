/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/fdt_wrappers.h>

#include <libfdt.h>
#include <fconf_nt_config_getter.h>

#include <plat/common/platform.h>

struct event_log_config_t event_log_config;

int fconf_populate_event_log_config(uintptr_t config)
{
	int err;
	int node;

	/* Necessary to work with libfdt APIs */
	const void *dtb = (const void *)config;

	/*
	 * Find the offset of the node containing "arm,tpm_event_log"
	 * compatible property
	 */
	const char *compatible_str = "arm,tpm_event_log";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find '%s' compatible in dtb\n",
			compatible_str);
		return node;
	}

	/* Retrieve Event Log details from the DTB */
#ifdef SPD_opteed
	err = fdtw_read_cells(dtb, node, "tpm_event_log_sm_addr", 2,
				&event_log_config.tpm_event_log_sm_addr);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'tpm_event_log_sm_addr'\n");
		return err;
	}
#endif
	err = fdtw_read_cells(dtb, node,
		"tpm_event_log_addr", 2, &event_log_config.tpm_event_log_addr);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'tpm_event_log_addr'\n");
		return err;
	}

	err = fdtw_read_cells(dtb, node,
		"tpm_event_log_size", 1, &event_log_config.tpm_event_log_size);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for 'tpm_event_log_size'\n");
	}

	return err;
}

FCONF_REGISTER_POPULATOR(NT_CONFIG, event_log_config,
				fconf_populate_event_log_config);
