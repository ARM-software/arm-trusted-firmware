/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
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
 * Read cells from a given property of the given node. Any number of 32-bit
 * cells of the property can be read. Returns 0 on success, or a negative
 * FDT error value otherwise.
 */
int fdt_read_uint32_array(const void *dtb, int node, const char *prop_name,
			  unsigned int cells, uint32_t *value)
{
	const fdt32_t *prop;
	int value_len;

	assert(dtb != NULL);
	assert(prop_name != NULL);
	assert(value != NULL);
	assert(node >= 0);

	/* Access property and obtain its length (in bytes) */
	prop = fdt_getprop(dtb, node, prop_name, &value_len);
	if (prop == NULL) {
		WARN("Couldn't find property %s in dtb\n", prop_name);
		return -FDT_ERR_NOTFOUND;
	}

	/* Verify that property length can fill the entire array. */
	if (NCELLS((unsigned int)value_len) < cells) {
		WARN("Property length mismatch\n");
		return -FDT_ERR_BADVALUE;
	}

	for (unsigned int i = 0U; i < cells; i++) {
		value[i] = fdt32_to_cpu(prop[i]);
	}

	return 0;
}

int fdt_read_uint32(const void *dtb, int node, const char *prop_name,
		    uint32_t *value)
{
	return fdt_read_uint32_array(dtb, node, prop_name, 1, value);
}

int fdt_read_uint64(const void *dtb, int node, const char *prop_name,
		    uint64_t *value)
{
	uint32_t array[2] = {0, 0};
	int ret;

	ret = fdt_read_uint32_array(dtb, node, prop_name, 2, array);
	if (ret < 0) {
		return ret;
	}

	*value = ((uint64_t)array[0] << 32) | array[1];
	return 0;
}

/*
 * Read bytes from a given property of the given node. Any number of
 * bytes of the property can be read. The fdt pointer is updated.
 * Returns 0 on success, and -1 on error.
 */
int fdtw_read_bytes(const void *dtb, int node, const char *prop,
		    unsigned int length, void *value)
{
	const void *ptr;
	int value_len;

	assert(dtb != NULL);
	assert(prop != NULL);
	assert(value != NULL);
	assert(node >= 0);

	/* Access property and obtain its length (in bytes) */
	ptr = fdt_getprop_namelen(dtb, node, prop, (int)strlen(prop),
					&value_len);
	if (ptr == NULL) {
		WARN("Couldn't find property %s in dtb\n", prop);
		return -1;
	}

	/* Verify that property length is not less than number of bytes */
	if ((unsigned int)value_len < length) {
		WARN("Property length mismatch\n");
		return -1;
	}

	(void)memcpy(value, ptr, length);

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

/*
 * Write bytes in place to a given property of the given node.
 * Any number of bytes of the property can be written.
 * Returns 0 on success, and < 0 on error.
 */
int fdtw_write_inplace_bytes(void *dtb, int node, const char *prop,
			     unsigned int length, const void *data)
{
	const void *ptr;
	int namelen, value_len, err;

	assert(dtb != NULL);
	assert(prop != NULL);
	assert(data != NULL);
	assert(node >= 0);

	namelen = (int)strlen(prop);

	/* Access property and obtain its length in bytes */
	ptr = fdt_getprop_namelen(dtb, node, prop, namelen, &value_len);
	if (ptr == NULL) {
		WARN("Couldn't find property %s in dtb\n", prop);
		return -1;
	}

	/* Verify that property length is not less than number of bytes */
	if ((unsigned int)value_len < length) {
		WARN("Property length mismatch\n");
		return -1;
	}

	/* Set property value in place */
	err = fdt_setprop_inplace_namelen_partial(dtb, node, prop,
						  namelen, 0,
						  data, (int)length);
	if (err != 0) {
		WARN("Set property %s failed with error %d\n", prop, err);
	}

	return err;
}
