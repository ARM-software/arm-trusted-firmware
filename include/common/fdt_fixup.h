/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FDT_FIXUP_H
#define FDT_FIXUP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INVALID_BASE_ADDR	((uintptr_t)~0UL)

struct psci_cpu_idle_state {
	const char *name;
	uint32_t power_state;
	bool local_timer_stop;
	uint32_t entry_latency_us;
	uint32_t exit_latency_us;
	uint32_t min_residency_us;
	uint32_t wakeup_latency_us;
};

int dt_add_psci_node(void *fdt);
int dt_add_psci_cpu_enable_methods(void *fdt);
int fdt_add_reserved_memory(void *dtb, const char *node_name,
			    uintptr_t base, size_t size);
int fdt_add_cpus_node(void *dtb, unsigned int afflv0,
		      unsigned int afflv1, unsigned int afflv2);
int fdt_add_cpu_idle_states(void *dtb, const struct psci_cpu_idle_state *state);
int fdt_adjust_gic_redist(void *dtb, unsigned int nr_cores, uintptr_t gicr_base,
			  unsigned int gicr_frame_size);
int fdt_set_mac_address(void *dtb, unsigned int ethernet_idx,
			const uint8_t *mac_addr);

#endif /* FDT_FIXUP_H */
