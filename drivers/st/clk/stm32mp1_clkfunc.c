/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dt-bindings/clock/stm32mp1-clksrc.h>
#include <errno.h>
#include <libfdt.h>
#include <stm32mp1_clk.h>
#include <stm32mp1_clkfunc.h>
#include <stm32mp1_dt.h>

#define DT_RCC_NODE_NAME	"rcc@50000000"
#define DT_RCC_CLK_COMPAT	"st,stm32mp1-rcc"
#define DT_RCC_COMPAT		"syscon"
#define DT_STGEN_COMPAT		"st,stm32-stgen"
#define DT_UART_COMPAT		"st,stm32h7-uart"
#define DT_USART_COMPAT		"st,stm32h7-usart"

const char *stm32mp_osc_node_label[NB_OSC] = {
	[_LSI] = "clk-lsi",
	[_LSE] = "clk-lse",
	[_HSI] = "clk-hsi",
	[_HSE] = "clk-hse",
	[_CSI] = "clk-csi",
	[_I2S_CKIN] = "i2s_ckin",
	[_USB_PHY_48] = "ck_usbo_48m"
};

/*******************************************************************************
 * This function reads the frequency of an oscillator from its name.
 * It reads the value indicated inside the device tree.
 * Returns 0 if success, and a negative value else.
 * If success, value is stored in the second parameter.
 ******************************************************************************/
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

/*******************************************************************************
 * This function checks the presence of an oscillator property from its id.
 * The search is done inside the device tree.
 * Returns true/false regarding search result.
 ******************************************************************************/
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

/*******************************************************************************
 * This function reads a value of a oscillator property from its id.
 * Returns value if success, and a default value if property not found.
 * Default value is passed as parameter.
 ******************************************************************************/
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

/*******************************************************************************
 * This function reads the rcc base address.
 * It reads the value indicated inside the device tree.
 * Returns address if success, and 0 value else.
 ******************************************************************************/
uint32_t fdt_rcc_read_addr(void)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return 0;
	}

	node = fdt_path_offset(fdt, "/soc");
	if (node < 0) {
		return 0;
	}

	fdt_for_each_subnode(subnode, fdt, node) {
		const char *cchar;
		int ret;

		cchar = fdt_get_name(fdt, subnode, &ret);
		if (cchar == NULL) {
			return 0;
		}

		if (strncmp(cchar, DT_RCC_NODE_NAME, (size_t)ret) == 0) {
			const fdt32_t *cuint;

			cuint = fdt_getprop(fdt, subnode, "reg", NULL);
			if (cuint == NULL) {
				return 0;
			}

			return fdt32_to_cpu(*cuint);
		}
	}

	return 0;
}

/*******************************************************************************
 * This function reads a series of parameters in rcc-clk section.
 * It reads the values indicated inside the device tree, from property name.
 * The number of parameters is also indicated as entry parameter.
 * Returns 0 if success, and a negative value else.
 * If success, values are stored at the second parameter address.
 ******************************************************************************/
int fdt_rcc_read_uint32_array(const char *prop_name,
			      uint32_t *array, uint32_t count)
{
	int node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_RCC_CLK_COMPAT);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return fdt_read_uint32_array(node, prop_name, array, count);
}

/*******************************************************************************
 * This function gets the subnode offset in rcc-clk section from its name.
 * It reads the values indicated inside the device tree.
 * Returns offset if success, and a negative value else.
 ******************************************************************************/
int fdt_rcc_subnode_offset(const char *name)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_RCC_CLK_COMPAT);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	subnode = fdt_subnode_offset(fdt, node, name);
	if (subnode <= 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return subnode;
}

/*******************************************************************************
 * This function gets the pointer to a rcc-clk property from its name.
 * It reads the values indicated inside the device tree.
 * Length of the property is stored in the second parameter.
 * Returns pointer if success, and NULL value else.
 ******************************************************************************/
const uint32_t *fdt_rcc_read_prop(const char *prop_name, int *lenp)
{
	const uint32_t *cuint;
	int node, len;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return NULL;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_RCC_CLK_COMPAT);
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

/*******************************************************************************
 * This function gets the secure status for rcc node.
 * It reads secure-status in device tree.
 * Returns 1 if rcc is available from secure world, 0 else.
 ******************************************************************************/
bool fdt_get_rcc_secure_status(void)
{
	int node;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return false;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_RCC_COMPAT);
	if (node < 0) {
		return false;
	}

	return fdt_check_secure_status(node);
}

/*******************************************************************************
 * This function reads the stgen base address.
 * It reads the value indicated inside the device tree.
 * Returns address if success, and NULL value else.
 ******************************************************************************/
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

/*******************************************************************************
 * This function gets the clock ID of the given node.
 * It reads the value indicated inside the device tree.
 * Returns ID if success, and a negative value else.
 ******************************************************************************/
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
