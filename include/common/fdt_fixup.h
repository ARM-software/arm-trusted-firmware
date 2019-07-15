/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FDT_FIXUP_H
#define FDT_FIXUP_H

int dt_add_psci_node(void *fdt);
int dt_add_psci_cpu_enable_methods(void *fdt);
int fdt_add_reserved_memory(void *dtb, const char *node_name,
			    uintptr_t base, size_t size);

#endif /* FDT_FIXUP_H */
