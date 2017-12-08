/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>
#include <ea_handle.h>
#include <ehf.h>
#include <platform.h>
#include <ras.h>
#include <ras_arch.h>

/* Handler that receives External Aborts on RAS-capable systems */
int ras_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
	unsigned int i, n_handled = 0, ret;
	int probe_data;
	struct err_record_info *info;

	const struct err_handler_data err_data = {
		.version = ERR_HANDLER_VERSION,
		.ea_reason = ea_reason,
		.syndrome = syndrome,
		.flags = flags,
		.cookie = cookie,
		.handle = handle
	};

	for_each_err_record_info(i, info) {
		assert(info->probe != NULL);
		assert(info->handler != NULL);

		/* Continue probing until the record group signals no error */
		while (1) {
			if (info->probe(info, &probe_data) == 0)
				break;

			/* Handle error */
			ret = info->handler(info, probe_data, &err_data);
			if (ret != 0)
				return ret;

			n_handled++;
		}
	}

	return (n_handled != 0);
}
