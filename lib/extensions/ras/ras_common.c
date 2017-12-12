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

#ifndef PLAT_RAS_PRI
# error Platform must define RAS priority value
#endif

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
		.interrupt = 0,
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

#if ENABLE_ASSERTIONS
static void assert_interrupts_sorted(void)
{
	unsigned int i, last;
	struct ras_interrupt *start = ras_interrupt_data.intrs;

	if (ras_interrupt_data.num_intrs == 0)
		return;

	last = start[0].intr_number;
	for (i = 1; i < ras_interrupt_data.num_intrs; i++) {
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
	struct ras_interrupt *start = ras_interrupt_data.intrs;
	struct ras_interrupt *selected = NULL;
	int s, e, m, probe_data, ret __unused;

	const struct err_handler_data err_data = {
		.version = ERR_HANDLER_VERSION,
		.interrupt = intr_raw,
		.flags = flags,
		.cookie = cookie,
		.handle = handle
	};

	assert(ras_interrupt_data.num_intrs > 0);

	s = 0;
	e = ras_interrupt_data.num_intrs;
	while (s <= e) {
		m = ((e + s) / 2);
		if (intr_raw == start[m].intr_number) {
			selected = &start[m];
			break;
		} else if (intr_raw < start[m].intr_number) {
			/* Move left */
			e = m - 1;
		} else {
			/* Move right */
			s = m + 1;
		}
	}

	if (selected == NULL) {
		ERROR("RAS interrupt %u has no handler!\n", intr_raw);
		panic();
	}


	ret = selected->info->probe(selected->info, &probe_data);
	assert(ret != 0);

	/* Call error handler for the record group */
	assert(selected->info->handler != NULL);
	selected->info->handler(selected->info, probe_data, &err_data);

	return 0;
}

void ras_init(void)
{
#if ENABLE_ASSERTIONS
	/* Sort RAS interrupts */
	assert_interrupts_sorted();
#endif

	/* Register RAS priority handler */
	ehf_register_priority_handler(PLAT_RAS_PRI, ras_interrupt_handler);
}
