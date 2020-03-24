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

uint32_t fdt_read_uint32_default(const void *dtb, int node,
				 const char *prop_name, uint32_t dflt_value)
{
	uint32_t ret = dflt_value;
	int err = fdt_read_uint32(dtb, node, prop_name, &ret);

	if (err < 0) {
		return dflt_value;
	}

	return ret;
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

static uint64_t fdt_read_prop_cells(const fdt32_t *prop, int nr_cells)
{
	uint64_t reg = fdt32_to_cpu(prop[0]);

	if (nr_cells > 1) {
		reg = (reg << 32) | fdt32_to_cpu(prop[1]);
	}

	return reg;
}

int fdt_get_reg_props_by_index(const void *dtb, int node, int index,
			       uintptr_t *base, size_t *size)
{
	const fdt32_t *prop;
	int parent, len;
	int ac, sc;
	int cell;

	parent = fdt_parent_offset(dtb, node);
	if (parent < 0) {
		return -FDT_ERR_BADOFFSET;
	}

	ac = fdt_address_cells(dtb, parent);
	sc = fdt_size_cells(dtb, parent);

	cell = index * (ac + sc);

	prop = fdt_getprop(dtb, node, "reg", &len);
	if (prop == NULL) {
		WARN("Couldn't find \"reg\" property in dtb\n");
		return -FDT_ERR_NOTFOUND;
	}

	if (((cell + ac + sc) * (int)sizeof(uint32_t)) > len) {
		return -FDT_ERR_BADVALUE;
	}

	if (base != NULL) {
		*base = (uintptr_t)fdt_read_prop_cells(&prop[cell], ac);
	}

	if (size != NULL) {
		*size = (size_t)fdt_read_prop_cells(&prop[cell + ac], sc);
	}

	return 0;
}

/*******************************************************************************
 * This function fills reg node info (base & size) with an index found by
 * checking the reg-names node.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int fdt_get_reg_props_by_name(const void *dtb, int node, const char *name,
			      uintptr_t *base, size_t *size)
{
	int index;

	index = fdt_stringlist_search(dtb, node, "reg-names", name);
	if (index < 0) {
		return index;
	}

	return fdt_get_reg_props_by_index(dtb, node, index, base, size);
}

/*******************************************************************************
 * This function gets the stdout path node.
 * It reads the value indicated inside the device tree.
 * Returns node offset on success and a negative FDT error code on failure.
 ******************************************************************************/
int fdt_get_stdout_node_offset(const void *dtb)
{
	int node;
	const char *prop, *path;
	int len;

	/* The /secure-chosen node takes precedence over the standard one. */
	node = fdt_path_offset(dtb, "/secure-chosen");
	if (node < 0) {
		node = fdt_path_offset(dtb, "/chosen");
		if (node < 0) {
			return -FDT_ERR_NOTFOUND;
		}
	}

	prop = fdt_getprop(dtb, node, "stdout-path", NULL);
	if (prop == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	/* Determine the actual path length, as a colon terminates the path. */
	path = strchr(prop, ':');
	if (path == NULL) {
		len = strlen(prop);
	} else {
		len = path - prop;
	}

	/* Aliases cannot start with a '/', so it must be the actual path. */
	if (prop[0] == '/') {
		return fdt_path_offset_namelen(dtb, prop, len);
	}

	/* Lookup the alias, as this contains the actual path. */
	path = fdt_get_alias_namelen(dtb, prop, len);
	if (path == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	return fdt_path_offset(dtb, path);
}


/*******************************************************************************
 * Only devices which are direct children of root node use CPU address domain.
 * All other devices use addresses that are local to the device node and cannot
 * directly used by CPU. Device tree provides an address translation mechanism
 * through "ranges" property which provides mappings from local address space to
 * parent address space. Since a device could be a child of a child node to the
 * root node, there can be more than one level of address translation needed to
 * map the device local address space to CPU address space.
 * fdtw_translate_address() API performs address translation of a local address
 * to a global address with help of various helper functions.
 ******************************************************************************/

static bool fdtw_xlat_hit(const uint32_t *value, int child_addr_size,
		int parent_addr_size, int range_size, uint64_t base_address,
		uint64_t *translated_addr)
{
	uint64_t local_address, parent_address, addr_range;

	local_address = fdt_read_prop_cells(value, child_addr_size);
	parent_address = fdt_read_prop_cells(value + child_addr_size,
				parent_addr_size);
	addr_range = fdt_read_prop_cells(value + child_addr_size +
				parent_addr_size,
				range_size);
	VERBOSE("DT: Address %llx mapped to %llx with range %llx\n",
		local_address, parent_address, addr_range);

	/* Perform range check */
	if ((base_address < local_address) ||
		(base_address >= local_address + addr_range)) {
		return false;
	}

	/* Found hit for the addr range that needs to be translated */
	*translated_addr = parent_address + (base_address - local_address);
	VERBOSE("DT: child address %llx mapped to %llx in parent bus\n",
			local_address, parent_address);
	return true;
}

#define ILLEGAL_ADDR	ULL(~0)

static uint64_t fdtw_search_all_xlat_entries(const void *dtb,
				const struct fdt_property *ranges_prop,
				int local_bus, uint64_t base_address)
{
	uint64_t translated_addr;
	const uint32_t *next_entry;
	int parent_bus_node, nxlat_entries, length;
	int self_addr_cells, parent_addr_cells, self_size_cells, ncells_xlat;

	/*
	 * The number of cells in one translation entry in ranges is the sum of
	 * the following values:
	 * self#address-cells + parent#address-cells + self#size-cells
	 * Ex: the iofpga ranges property has one translation entry with 4 cells
	 * They represent iofpga#addr-cells + motherboard#addr-cells + iofpga#size-cells
	 *              = 1                 + 2                      + 1
	 */

	parent_bus_node = fdt_parent_offset(dtb, local_bus);
	self_addr_cells = fdt_address_cells(dtb, local_bus);
	self_size_cells = fdt_size_cells(dtb, local_bus);
	parent_addr_cells = fdt_address_cells(dtb, parent_bus_node);

	/* Number of cells per translation entry i.e., mapping */
	ncells_xlat = self_addr_cells + parent_addr_cells + self_size_cells;

	assert(ncells_xlat > 0);

	/*
	 * Find the number of translations(mappings) specified in the current
	 * `ranges` property. Note that length represents number of bytes and
	 * is stored in big endian mode.
	 */
	length = fdt32_to_cpu(ranges_prop->len);
	nxlat_entries = (length/sizeof(uint32_t))/ncells_xlat;

	assert(nxlat_entries > 0);

	next_entry = (const uint32_t *)ranges_prop->data;

	/* Iterate over the entries in the "ranges" */
	for (int i = 0; i < nxlat_entries; i++) {
		if (fdtw_xlat_hit(next_entry, self_addr_cells,
				parent_addr_cells, self_size_cells, base_address,
				&translated_addr)){
			return translated_addr;
		}
		next_entry = next_entry + ncells_xlat;
	}

	INFO("DT: No translation found for address %llx in node %s\n",
		base_address, fdt_get_name(dtb, local_bus, NULL));
	return ILLEGAL_ADDR;
}


/*******************************************************************************
 * address mapping needs to be done recursively starting from current node to
 * root node through all intermediate parent nodes.
 * Sample device tree is shown here:

smb@0,0 {
	compatible = "simple-bus";

	#address-cells = <2>;
	#size-cells = <1>;
	ranges = <0 0 0 0x08000000 0x04000000>,
		 <1 0 0 0x14000000 0x04000000>,
		 <2 0 0 0x18000000 0x04000000>,
		 <3 0 0 0x1c000000 0x04000000>,
		 <4 0 0 0x0c000000 0x04000000>,
		 <5 0 0 0x10000000 0x04000000>;

	motherboard {
		arm,v2m-memory-map = "rs1";
		compatible = "arm,vexpress,v2m-p1", "simple-bus";
		#address-cells = <2>;
		#size-cells = <1>;
		ranges;

		iofpga@3,00000000 {
			compatible = "arm,amba-bus", "simple-bus";
			#address-cells = <1>;
			#size-cells = <1>;
			ranges = <0 3 0 0x200000>;
			v2m_serial1: uart@a0000 {
				compatible = "arm,pl011", "arm,primecell";
				reg = <0x0a0000 0x1000>;
				interrupts = <0 6 4>;
				clocks = <&v2m_clk24mhz>, <&v2m_clk24mhz>;
				clock-names = "uartclk", "apb_pclk";
		};
	};
};

 * As seen above, there are 3 levels of address translations needed. An empty
 * `ranges` property denotes identity mapping (as seen in `motherboard` node).
 * Each ranges property can map a set of child addresses to parent bus. Hence
 * there can be more than 1 (translation) entry in the ranges property as seen
 * in the `smb` node which has 6 translation entries.
 ******************************************************************************/

/* Recursive implementation */
uint64_t fdtw_translate_address(const void *dtb, int node,
				uint64_t base_address)
{
	int length, local_bus_node;
	const char *node_name;
	uint64_t global_address;

	local_bus_node = fdt_parent_offset(dtb, node);
	node_name = fdt_get_name(dtb, local_bus_node, NULL);

	/*
	 * In the example given above, starting from the leaf node:
	 * uart@a000 represents the current node
	 * iofpga@3,00000000 represents the local bus
	 * motherboard represents the parent bus
	 */

	/* Read the ranges property */
	const struct fdt_property *property = fdt_get_property(dtb,
					local_bus_node, "ranges", &length);

	if (property == NULL) {
		if (local_bus_node == 0) {
			/*
			 * root node doesn't have range property as addresses
			 * are in CPU address space.
			 */
			return base_address;
		}
		INFO("DT: Couldn't find ranges property in node %s\n",
			node_name);
		return ILLEGAL_ADDR;
	} else if (length == 0) {
		/* empty ranges indicates identity map to parent bus */
		return fdtw_translate_address(dtb, local_bus_node, base_address);
	}

	VERBOSE("DT: Translation lookup in node %s at offset %d\n", node_name,
		local_bus_node);
	global_address = fdtw_search_all_xlat_entries(dtb, property,
				local_bus_node, base_address);

	if (global_address == ILLEGAL_ADDR) {
		return ILLEGAL_ADDR;
	}

	/* Translate the local device address recursively */
	return fdtw_translate_address(dtb, local_bus_node, global_address);
}
