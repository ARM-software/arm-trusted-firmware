/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <libfdt.h>

#include <platform_def.h>

#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp_clkfunc.h>

#define DT_STGEN_COMPAT		"st,stm32-stgen"

/*
 * Get the frequency of an oscillator from its name in device tree.
 * @param name: oscillator name
 * @param freq: stores the frequency of the oscillator
 * @return: 0 on success, and a negative FDT/ERRNO error code on failure.
 */
int fdt_osc_read_freq(const char *name, uint32_t *freq)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_path_offset(fdt, "/clocks");
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	fdt_for_each_subnode(subnode, fdt, node) {
		const char *cchar;
		int ret;

		cchar = fdt_get_name(fdt, subnode, &ret);
		if (cchar == NULL) {
			return ret;
		}

		if (strncmp(cchar, name, (size_t)ret) == 0) {
			const fdt32_t *cuint;

			cuint = fdt_getprop(fdt, subnode, "clock-frequency",
					    &ret);
			if (cuint == NULL) {
				return ret;
			}

			*freq = fdt32_to_cpu(*cuint);

			return 0;
		}
	}

	/* Oscillator not found, freq=0 */
	*freq = 0;
	return 0;
}

/*
 * Check the presence of an oscillator property from its id.
 * @param osc_id: oscillator ID
 * @param prop_name: property name
 * @return: true/false regarding search result.
 */
bool fdt_osc_read_bool(enum stm32mp_osc_id osc_id, const char *prop_name)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return false;
	}

	if (osc_id >= NB_OSC) {
		return false;
	}

	node = fdt_path_offset(fdt, "/clocks");
	if (node < 0) {
		return false;
	}

	fdt_for_each_subnode(subnode, fdt, node) {
		const char *cchar;
		int ret;

		cchar = fdt_get_name(fdt, subnode, &ret);
		if (cchar == NULL) {
			return false;
		}

		if (strncmp(cchar, stm32mp_osc_node_label[osc_id],
			    (size_t)ret) != 0) {
			continue;
		}

		if (fdt_getprop(fdt, subnode, prop_name, NULL) != NULL) {
			return true;
		}
	}

	return false;
}

/*
 * Get the value of a oscillator property from its ID.
 * @param osc_id: oscillator ID
 * @param prop_name: property name
 * @param dflt_value: default value
 * @return oscillator value on success, default value if property not found.
 */
uint32_t fdt_osc_read_uint32_default(enum stm32mp_osc_id osc_id,
				     const char *prop_name, uint32_t dflt_value)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return dflt_value;
	}

	if (osc_id >= NB_OSC) {
		return dflt_value;
	}

	node = fdt_path_offset(fdt, "/clocks");
	if (node < 0) {
		return dflt_value;
	}

	fdt_for_each_subnode(subnode, fdt, node) {
		const char *cchar;
		int ret;

		cchar = fdt_get_name(fdt, subnode, &ret);
		if (cchar == NULL) {
			return dflt_value;
		}

		if (strncmp(cchar, stm32mp_osc_node_label[osc_id],
			    (size_t)ret) != 0) {
			continue;
		}

		return fdt_read_uint32_default(subnode, prop_name, dflt_value);
	}

	return dflt_value;
}

/*
 * Get the RCC node offset from the device tree
 * @param fdt: Device tree reference
 * @return: Node offset or a negative value on error
 */
int fdt_get_rcc_node(void *fdt)
{
	return fdt_node_offset_by_compatible(fdt, -1, DT_RCC_CLK_COMPAT);
}

/*
 * Get the RCC base address from the device tree
 * @return: RCC address or 0 on error
 */
uint32_t fdt_rcc_read_addr(void)
{
	int node;
	void *fdt;
	const fdt32_t *cuint;

	if (fdt_get_address(&fdt) == 0) {
		return 0;
	}

	node = fdt_get_rcc_node(fdt);
	if (node < 0) {
		return 0;
	}

	cuint = fdt_getprop(fdt, node, "reg", NULL);
	if (cuint == NULL) {
		return 0;
	}

	return fdt32_to_cpu(*cuint);
}

/*
 * Read a series of parameters in rcc-clk section in device tree
 * @param prop_name: Name of the RCC property to be read
 * @param array: the array to store the property parameters
 * @param count: number of parameters to be read
 * @return: 0 on succes or a negative value on error
 */
int fdt_rcc_read_uint32_array(const char *prop_name,
			      uint32_t *array, uint32_t count)
{
	int node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_get_rcc_node(fdt);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return fdt_read_uint32_array(node, prop_name, array, count);
}

/*
 * Get the subnode offset in rcc-clk section from its name in device tree
 * @param name: name of the RCC property
 * @return: offset on success, and a negative FDT/ERRNO error code on failure.
 */
int fdt_rcc_subnode_offset(const char *name)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_get_rcc_node(fdt);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	subnode = fdt_subnode_offset(fdt, node, name);
	if (subnode <= 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return subnode;
}

/*
 * Get the pointer to a rcc-clk property from its name.
 * @param name: name of the RCC property
 * @param lenp: stores the length of the property.
 * @return: pointer to the property on success, and NULL value on failure.
 */
const fdt32_t *fdt_rcc_read_prop(const char *prop_name, int *lenp)
{
	const fdt32_t *cuint;
	int node, len;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return NULL;
	}

	node = fdt_get_rcc_node(fdt);
	if (node < 0) {
		return NULL;
	}

	cuint = fdt_getprop(fdt, node, prop_name, &len);
	if (cuint == NULL) {
		return NULL;
	}

	*lenp = len;
	return cuint;
}

/*
 * Get the secure status for rcc node in device tree.
 * @return: true if rcc is available from secure world, false if not.
 */
bool fdt_get_rcc_secure_status(void)
{
	int node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return false;
	}

	node = fdt_get_rcc_node(fdt);
	if (node < 0) {
		return false;
	}

	return !!(fdt_get_status(node) & DT_SECURE);
}

/*
 * Get the stgen base address.
 * @return: address of stgen on success, and NULL value on failure.
 */
uintptr_t fdt_get_stgen_base(void)
{
	int node;
	const fdt32_t *cuint;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return 0;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_STGEN_COMPAT);
	if (node < 0) {
		return 0;
	}

	cuint = fdt_getprop(fdt, node, "reg", NULL);
	if (cuint == NULL) {
		return 0;
	}

	return fdt32_to_cpu(*cuint);
}

/*
 * Get the clock ID of the given node in device tree.
 * @param node: node offset
 * @return: Clock ID on success, and a negative FDT/ERRNO error code on failure.
 */
int fdt_get_clock_id(int node)
{
	const fdt32_t *cuint;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	cuint = fdt_getprop(fdt, node, "clocks", NULL);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint++;
	return (int)fdt32_to_cpu(*cuint);
}
