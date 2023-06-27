/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Helper functions to offer easier navigation of Device Tree Blob */

#ifndef FDT_WRAPPERS_H
#define FDT_WRAPPERS_H

#include <libfdt_env.h>
#include <libfdt.h>

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

int fdtw_for_each_cpu(const void *fdt,
		      int (*callback)(const void *dtb, int node, uintptr_t mpidr));

int fdtw_find_or_add_subnode(void *fdt, int parentoffset, const char *name);

static inline uint32_t fdt_blob_size(const void *dtb)
{
	const uint32_t *dtb_header = (const uint32_t *)dtb;

	return fdt32_to_cpu(dtb_header[1]);
}

static inline bool fdt_node_is_enabled(const void *fdt, int node)
{
	int len;
	const void *prop = fdt_getprop(fdt, node, "status", &len);

	/* A non-existing status property means the device is enabled. */
	return (prop == NULL) || (len == 5 && strcmp((const char *)prop,
		"okay") == 0);
}

#define fdt_for_each_compatible_node(dtb, node, compatible_str)       \
for (node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);   \
     node >= 0;                                                       \
     node = fdt_node_offset_by_compatible(dtb, node, compatible_str))

#endif /* FDT_WRAPPERS_H */
