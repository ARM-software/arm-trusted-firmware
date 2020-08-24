/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FDT_FIXUP_H
#define FDT_FIXUP_H

int dt_add_psci_node(void *fdt);
int dt_add_psci_cpu_enable_methods(void *fdt);
int fdt_add_reserved_memory(void *dtb, const char *node_name,
			    uintptr_t base, size_t size);
int fdt_add_cpus_node(void *dtb, unsigned int afflv0,
		      unsigned int afflv1, unsigned int afflv2);
int fdt_adjust_gic_redist(void *dtb, unsigned int nr_cores,
			  unsigned int gicr_frame_size);

#endif /* FDT_FIXUP_H */
