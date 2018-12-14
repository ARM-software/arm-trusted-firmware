/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Helper functions to offer easier navigation of Device Tree Blob */

#include <assert.h>
#include <string.h>

#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>

/*
 * Read cells from a given property of the given node. At most 2 cells of the
 * property are read, and pointer is updated. Returns 0 on success, and -1 upon
 * error
 */
int fdtw_read_cells(const void *dtb, int node, const char *prop,
		unsigned int cells, void *value)
{
	const uint32_t *value_ptr;
	uint32_t hi = 0, lo;
	int value_len;

	assert(dtb != NULL);
	assert(prop != NULL);
	assert(value != NULL);
	assert(node >= 0);

	/* We expect either 1 or 2 cell property */
	assert(cells <= 2U);

	/* Access property and obtain its length (in bytes) */
	value_ptr = fdt_getprop_namelen(dtb, node, prop, (int)strlen(prop),
			&value_len);
	if (value_ptr == NULL) {
		WARN("Couldn't find property %s in dtb\n", prop);
		return -1;
	}

	/* Verify that property length accords with cell length */
	if (NCELLS((unsigned int)value_len) != cells) {
		WARN("Property length mismatch\n");
		return -1;
	}

	if (cells == 2U) {
		hi = fdt32_to_cpu(*value_ptr);
		value_ptr++;
	}

	lo = fdt32_to_cpu(*value_ptr);

	if (cells == 2U)
		*((uint64_t *) value) = ((uint64_t) hi << 32) | lo;
	else
		*((uint32_t *) value) = lo;

	return 0;
}

/*
 * Read cells from a given property of the given node. Any number of 32-bit
 * cells of the property can be read. The fdt pointer is updated. Returns 0 on
 * success, and -1 on error.
 */
int fdtw_read_array(const void *dtb, int node, const char *prop,
		unsigned int cells, void *value)
{
	const uint32_t *value_ptr;
	int value_len;

	assert(dtb != NULL);
	assert(prop != NULL);
	assert(value != NULL);
	assert(node >= 0);

	/* Access property and obtain its length (in bytes) */
	value_ptr = fdt_getprop_namelen(dtb, node, prop, (int)strlen(prop),
			&value_len);
	if (value_ptr == NULL) {
		WARN("Couldn't find property %s in dtb\n", prop);
		return -1;
	}

	/* Verify that property length accords with cell length */
	if (NCELLS((unsigned int)value_len) != cells) {
		WARN("Property length mismatch\n");
		return -1;
	}

	uint32_t *dst = value;

	for (unsigned int i = 0U; i < cells; i++) {
		dst[i] = fdt32_to_cpu(value_ptr[i]);
	}

	return 0;
}

/*
 * Read string from a given property of the given node. Up to 'size - 1'
 * characters are read, and a NUL terminator is added. Returns 0 on success,
 * and -1 upon error.
 */
int fdtw_read_string(const void *dtb, int node, const char *prop,
		char *str, size_t size)
{
	const char *ptr;
	size_t len;

	assert(dtb != NULL);
	assert(node >= 0);
	assert(prop != NULL);
	assert(str != NULL);
	assert(size > 0U);

	ptr = fdt_getprop_namelen(dtb, node, prop, (int)strlen(prop), NULL);
	if (ptr == NULL) {
		WARN("Couldn't find property %s in dtb\n", prop);
		return -1;
	}

	len = strlcpy(str, ptr, size);
	if (len >= size) {
		WARN("String of property %s in dtb has been truncated\n", prop);
		return -1;
	}

	return 0;
}

/*
 * Write cells in place to a given property of the given node. At most 2 cells
 * of the property are written. Returns 0 on success, and -1 upon error.
 */
int fdtw_write_inplace_cells(void *dtb, int node, const char *prop,
		unsigned int cells, void *value)
{
	int err, len;

	assert(dtb != NULL);
	assert(prop != NULL);
	assert(value != NULL);
	assert(node >= 0);

	/* We expect either 1 or 2 cell property */
	assert(cells <= 2U);

	if (cells == 2U)
		*(uint64_t *)value = cpu_to_fdt64(*(uint64_t *)value);
	else
		*(uint32_t *)value = cpu_to_fdt32(*(uint32_t *)value);

	len = (int)cells * 4;

	/* Set property value in place */
	err = fdt_setprop_inplace(dtb, node, prop, value, len);
	if (err != 0) {
		WARN("Modify property %s failed with error %d\n", prop, err);
		return -1;
	}

	return 0;
}
