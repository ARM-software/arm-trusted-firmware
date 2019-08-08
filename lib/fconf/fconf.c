/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <libfdt.h>
#include <platform_def.h>

void fconf_populate(uintptr_t config)
{
	assert(config != 0UL);

	/* Check if the pointer to DTB is correct */
	if (fdt_check_header((void *)config) != 0) {
		ERROR("FCONF: Invalid DTB file passed for FW_CONFIG\n");
		panic();
	}

	INFO("FCONF: Reading firmware configuration file from: 0x%lx\n", config);

	/* Go through all registered populate functions */
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_START__, start);
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_END__, end);
	const struct fconf_populator *populator;

	for (populator = start; populator != end; populator++) {
		assert((populator->info != NULL) && (populator->populate != NULL));

		INFO("FCONF: Reading firmware configuration information for: %s\n", populator->info);
		if (populator->populate(config) != 0) {
			/* TODO: handle property miss */
			panic();
		}
	}
}
