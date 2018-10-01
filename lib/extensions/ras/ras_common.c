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
#include <stdbool.h>

#ifndef PLAT_RAS_PRI
# error Platform must define RAS priority value
#endif

/* Handler that receives External Aborts on RAS-capable systems */
int ras_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
	unsigned int i, n_handled = 0;
	int probe_data, ret;
	struct err_record_info *info;

	const struct err_handler_data err_data = {
		.version = ERR_HANDLER_VERSION,
		.ea_reason = ea_reason,
		.interrupt = 0,
		.syndrome = (uint32_t) syndrome,
		.flags = flags,
		.cookie = cookie,
		.handle = handle
	};

	for_each_err_record_info(i, info) {
		assert(info->probe != NULL);
		assert(info->handler != NULL);

		/* Continue probing until the record group signals no error */
		while (true) {
			if (info->probe(info, &probe_data) == 0)
				break;

			/* Handle error */
			ret = info->handler(info, probe_data, &err_data);
			if (ret != 0)
				return ret;

			n_handled++;
		}
	}

	return (n_handled != 0U) ? 1 : 0;
}

#if ENABLE_ASSERTIONS
static void assert_interrupts_sorted(void)
{
	unsigned int i, last;
	struct ras_interrupt *start = ras_interrupt_mappings.intrs;

	if (ras_interrupt_mappings.num_intrs == 0UL)
		return;

	last = start[0].intr_number;
	for (i = 1; i < ras_interrupt_mappings.num_intrs; i++) {
		assert(start[i].intr_number > last);
		last = start[i].intr_number;
	}
}
#endif

/*
 * Given an RAS interrupt number, locate the registered handler and call it. If
 * no handler was found for the interrupt number, this function panics.
 */
static int ras_interrupt_handler(uint32_t intr_raw, uint32_t flags,
		void *handle, void *cookie)
{
	struct ras_interrupt *ras_inrs = ras_interrupt_mappings.intrs;
	struct ras_interrupt *selected = NULL;
	int start, end, mid, probe_data, ret __unused;

	const struct err_handler_data err_data = {
		.version = ERR_HANDLER_VERSION,
		.interrupt = intr_raw,
		.flags = flags,
		.cookie = cookie,
		.handle = handle
	};

	assert(ras_interrupt_mappings.num_intrs > 0UL);

	start = 0;
	end = (int) ras_interrupt_mappings.num_intrs;
	while (start <= end) {
		mid = ((end + start) / 2);
		if (intr_raw == ras_inrs[mid].intr_number) {
			selected = &ras_inrs[mid];
			break;
		} else if (intr_raw < ras_inrs[mid].intr_number) {
			/* Move left */
			end = mid - 1;
		} else {
			/* Move right */
			start = mid + 1;
		}
	}

	if (selected == NULL) {
		ERROR("RAS interrupt %u has no handler!\n", intr_raw);
		panic();
	}

	if (selected->err_record->probe != NULL) {
		ret = selected->err_record->probe(selected->err_record, &probe_data);
		assert(ret != 0);
	}

	/* Call error handler for the record group */
	assert(selected->err_record->handler != NULL);
	(void) selected->err_record->handler(selected->err_record, probe_data,
			&err_data);

	return 0;
}

void __init ras_init(void)
{
#if ENABLE_ASSERTIONS
	/* Check RAS interrupts are sorted */
	assert_interrupts_sorted();
#endif

	/* Register RAS priority handler */
	ehf_register_priority_handler(PLAT_RAS_PRI, ras_interrupt_handler);
}
