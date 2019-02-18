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
