/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <libfdt.h>
#include <platform_def.h>
#include <stm32mp1_clk.h>
#include <stm32mp1_clkfunc.h>
#include <stm32mp1_dt.h>

#define DT_GPIO_BANK_SHIFT	12
#define DT_GPIO_BANK_MASK	0x1F000U
#define DT_GPIO_PIN_SHIFT	8
#define DT_GPIO_PIN_MASK	0xF00U
#define DT_GPIO_MODE_MASK	0xFFU

static int fdt_checked;

static void *fdt = (void *)(uintptr_t)STM32MP1_DTB_BASE;

/*******************************************************************************
 * This function checks device tree file with its header.
 * Returns 0 if success, and a negative value else.
 ******************************************************************************/
int dt_open_and_check(void)
{
	int ret = fdt_check_header(fdt);

	if (ret == 0) {
		fdt_checked = 1;
	}

	return ret;
}

/*******************************************************************************
 * This function gets the address of the DT.
 * If DT is OK, fdt_addr is filled with DT address.
 * Returns 1 if success, 0 otherwise.
 ******************************************************************************/
int fdt_get_address(void **fdt_addr)
{
	if (fdt_checked == 1) {
		*fdt_addr = fdt;
	}

	return fdt_checked;
}

/*******************************************************************************
 * This function check the presence of a node (generic use of fdt library).
 * Returns true if present, false else.
 ******************************************************************************/
bool fdt_check_node(int node)
{
	int len;
	const char *cchar;

	cchar = fdt_get_name(fdt, node, &len);

	return (cchar != NULL) && (len >= 0);
}

/*******************************************************************************
 * This function check the status of a node (generic use of fdt library).
 * Returns true if "okay" or missing, false else.
 ******************************************************************************/
bool fdt_check_status(int node)
{
	int len;
	const char *cchar;

	cchar = fdt_getprop(fdt, node, "status", &len);
	if (cchar == NULL) {
		return true;
	}

	return strncmp(cchar, "okay", (size_t)len) == 0;
}

/*******************************************************************************
 * This function check the secure-status of a node (generic use of fdt library).
 * Returns true if "okay" or missing, false else.
 ******************************************************************************/
bool fdt_check_secure_status(int node)
{
	int len;
	const char *cchar;

	cchar = fdt_getprop(fdt, node, "secure-status", &len);
	if (cchar == NULL) {
		return true;
	}

	return strncmp(cchar, "okay", (size_t)len) == 0;
}

/*******************************************************************************
 * This function reads a value of a node property (generic use of fdt
 * library).
 * Returns value if success, and a default value if property not found.
 * Default value is passed as parameter.
 ******************************************************************************/
uint32_t fdt_read_uint32_default(int node, const char *prop_name,
				 uint32_t dflt_value)
{
	const fdt32_t *cuint;
	int lenp;

	cuint = fdt_getprop(fdt, node, prop_name, &lenp);
	if (cuint == NULL) {
		return dflt_value;
	}

	return fdt32_to_cpu(*cuint);
}

/*******************************************************************************
 * This function reads a series of parameters in a node property
 * (generic use of fdt library).
 * It reads the values inside the device tree, from property name and node.
 * The number of parameters is also indicated as entry parameter.
 * Returns 0 if success, and a negative value else.
 * If success, values are stored at the third parameter address.
 ******************************************************************************/
int fdt_read_uint32_array(int node, const char *prop_name, uint32_t *array,
			  uint32_t count)
{
	const fdt32_t *cuint;
	int len;
	uint32_t i;

	cuint = fdt_getprop(fdt, node, prop_name, &len);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	if ((uint32_t)len != (count * sizeof(uint32_t))) {
		return -FDT_ERR_BADLAYOUT;
	}

	for (i = 0; i < ((uint32_t)len / sizeof(uint32_t)); i++) {
		*array = fdt32_to_cpu(*cuint);
		array++;
		cuint++;
	}

	return 0;
}
