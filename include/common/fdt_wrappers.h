/*
 * Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Helper functions to offer easier navigation of Device Tree Blob */

#ifndef FDT_WRAPPERS_H
#define FDT_WRAPPERS_H

#include <libfdt_env.h>

/* Number of cells, given total length in bytes. Each cell is 4 bytes long */
#define NCELLS(len) ((len) / 4U)

int fdt_read_uint32(const void *dtb, int node, const char *prop_name,
		    uint32_t *value);
uint32_t fdt_read_uint32_default(const void *dtb, int node,
				 const char *prop_name, uint32_t dflt_value);
int fdt_read_uint64(const void *dtb, int node, const char *prop_name,
		    uint64_t *value);
int fdt_read_uint32_array(const void *dtb, int node, const char *prop_name,
			  unsigned int cells, uint32_t *value);
int fdtw_read_string(const void *dtb, int node, const char *prop,
		char *str, size_t size);
int fdtw_read_uuid(const void *dtb, int node, const char *prop,
		   unsigned int length, uint8_t *uuid);
int fdtw_write_inplace_cells(void *dtb, int node, const char *prop,
		unsigned int cells, void *value);
int fdtw_read_bytes(const void *dtb, int node, const char *prop,
		unsigned int length, void *value);
int fdtw_write_inplace_bytes(void *dtb, int node, const char *prop,
		unsigned int length, const void *data);
int fdt_get_reg_props_by_index(const void *dtb, int node, int index,
			       uintptr_t *base, size_t *size);
int fdt_get_reg_props_by_name(const void *dtb, int node, const char *name,
			      uintptr_t *base, size_t *size);
int fdt_get_stdout_node_offset(const void *dtb);

uint64_t fdtw_translate_address(const void *dtb, int bus_node,
				uint64_t base_address);

static inline uint32_t fdt_blob_size(const void *dtb)
{
	const uint32_t *dtb_header = dtb;

	return fdt32_to_cpu(dtb_header[1]);
}

#endif /* FDT_WRAPPERS_H */
