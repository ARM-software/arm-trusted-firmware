/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp1_ddr.h>
#include <drivers/st/stm32mp1_ram.h>

#include <stm32mp_dt.h>

static int fdt_checked;

static void *fdt = (void *)(uintptr_t)STM32MP_DTB_BASE;

/*******************************************************************************
 * This function checks device tree file with its header.
 * Returns 0 on success and a negative FDT error code on failure.
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
 * Returns true if present, else return false.
 ******************************************************************************/
bool fdt_check_node(int node)
{
	int len;
	const char *cchar;

	cchar = fdt_get_name(fdt, node, &len);

	return (cchar != NULL) && (len >= 0);
}

/*******************************************************************************
 * This function return global node status (generic use of fdt library).
 ******************************************************************************/
uint8_t fdt_get_status(int node)
{
	uint8_t status = DT_DISABLED;
	int len;
	const char *cchar;

	cchar = fdt_getprop(fdt, node, "status", &len);
	if ((cchar == NULL) ||
	    (strncmp(cchar, "okay", (size_t)len) == 0)) {
		status |= DT_NON_SECURE;
	}

	cchar = fdt_getprop(fdt, node, "secure-status", &len);
	if (cchar == NULL) {
		if (status == DT_NON_SECURE) {
			status |= DT_SECURE;
		}
	} else if (strncmp(cchar, "okay", (size_t)len) == 0) {
		status |= DT_SECURE;
	}

	return status;
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
 * Returns 0 on success and a negative FDT error code on failure.
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

/*******************************************************************************
 * This function gets the stdout path node.
 * It reads the value indicated inside the device tree.
 * Returns node offset on success and a negative FDT error code on failure.
 ******************************************************************************/
static int dt_get_stdout_node_offset(void)
{
	int node;
	const char *cchar;

	node = fdt_path_offset(fdt, "/secure-chosen");
	if (node < 0) {
		node = fdt_path_offset(fdt, "/chosen");
		if (node < 0) {
			return -FDT_ERR_NOTFOUND;
		}
	}

	cchar = fdt_getprop(fdt, node, "stdout-path", NULL);
	if (cchar == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	node = -FDT_ERR_NOTFOUND;
	if (strchr(cchar, (int)':') != NULL) {
		const char *name;
		char *str = (char *)cchar;
		int len = 0;

		while (strncmp(":", str, 1)) {
			len++;
			str++;
		}

		name = fdt_get_alias_namelen(fdt, cchar, len);

		if (name != NULL) {
			node = fdt_path_offset(fdt, name);
		}
	} else {
		node = fdt_path_offset(fdt, cchar);
	}

	return node;
}

/*******************************************************************************
 * This function gets the stdout pin configuration information from the DT.
 * And then calls the sub-function to treat it and set GPIO registers.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_set_stdout_pinctrl(void)
{
	int node;

	node = dt_get_stdout_node_offset();
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return dt_set_pinctrl_config(node);
}

/*******************************************************************************
 * This function fills the generic information from a given node.
 ******************************************************************************/
void dt_fill_device_info(struct dt_node_info *info, int node)
{
	const fdt32_t *cuint;

	cuint = fdt_getprop(fdt, node, "reg", NULL);
	if (cuint != NULL) {
		info->base = fdt32_to_cpu(*cuint);
	} else {
		info->base = 0;
	}

	cuint = fdt_getprop(fdt, node, "clocks", NULL);
	if (cuint != NULL) {
		cuint++;
		info->clock = (int)fdt32_to_cpu(*cuint);
	} else {
		info->clock = -1;
	}

	cuint = fdt_getprop(fdt, node, "resets", NULL);
	if (cuint != NULL) {
		cuint++;
		info->reset = (int)fdt32_to_cpu(*cuint);
	} else {
		info->reset = -1;
	}

	info->status = fdt_get_status(node);
}

/*******************************************************************************
 * This function retrieve the generic information from DT.
 * Returns node on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_get_node(struct dt_node_info *info, int offset, const char *compat)
{
	int node;

	node = fdt_node_offset_by_compatible(fdt, offset, compat);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	dt_fill_device_info(info, node);

	return node;
}

/*******************************************************************************
 * This function gets the UART instance info of stdout from the DT.
 * Returns node on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_get_stdout_uart_info(struct dt_node_info *info)
{
	int node;

	node = dt_get_stdout_node_offset();
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	dt_fill_device_info(info, node);

	return node;
}

/*******************************************************************************
 * This function gets DDR size information from the DT.
 * Returns value in bytes on success, and 0 on failure.
 ******************************************************************************/
uint32_t dt_get_ddr_size(void)
{
	int node;

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read DDR node in DT\n", __func__);
		return 0;
	}

	return fdt_read_uint32_default(node, "st,mem-size", 0);
}

/*******************************************************************************
 * This function gets DDRCTRL base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
uintptr_t dt_get_ddrctrl_base(void)
{
	int node;
	uint32_t array[4];

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read DDR node in DT\n", __func__);
		return 0;
	}

	if (fdt_read_uint32_array(node, "reg", array, 4) < 0) {
		return 0;
	}

	return array[0];
}

/*******************************************************************************
 * This function gets DDRPHYC base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
uintptr_t dt_get_ddrphyc_base(void)
{
	int node;
	uint32_t array[4];

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read DDR node in DT\n", __func__);
		return 0;
	}

	if (fdt_read_uint32_array(node, "reg", array, 4) < 0) {
		return 0;
	}

	return array[2];
}

/*******************************************************************************
 * This function gets PWR base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
uintptr_t dt_get_pwr_base(void)
{
	int node;
	const fdt32_t *cuint;

	node = fdt_node_offset_by_compatible(fdt, -1, DT_PWR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read PWR node in DT\n", __func__);
		return 0;
	}

	cuint = fdt_getprop(fdt, node, "reg", NULL);
	if (cuint == NULL) {
		return 0;
	}

	return fdt32_to_cpu(*cuint);
}

/*******************************************************************************
 * This function gets PWR VDD regulator voltage information from the DT.
 * Returns value in microvolts on success, and 0 on failure.
 ******************************************************************************/
uint32_t dt_get_pwr_vdd_voltage(void)
{
	int node, pwr_regulators_node;
	const fdt32_t *cuint;

	node = fdt_node_offset_by_compatible(fdt, -1, DT_PWR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read PWR node in DT\n", __func__);
		return 0;
	}

	pwr_regulators_node = fdt_subnode_offset(fdt, node, "pwr-regulators");
	if (node < 0) {
		INFO("%s: Cannot read pwr-regulators node in DT\n", __func__);
		return 0;
	}

	cuint = fdt_getprop(fdt, pwr_regulators_node, "vdd-supply", NULL);
	if (cuint == NULL) {
		return 0;
	}

	node = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
	if (node < 0) {
		return 0;
	}

	cuint = fdt_getprop(fdt, node, "regulator-min-microvolt", NULL);
	if (cuint == NULL) {
		return 0;
	}

	return fdt32_to_cpu(*cuint);
}

/*******************************************************************************
 * This function gets SYSCFG base address information from the DT.
 * Returns value on success, and 0 on failure.
 ******************************************************************************/
uintptr_t dt_get_syscfg_base(void)
{
	int node;
	const fdt32_t *cuint;

	node = fdt_node_offset_by_compatible(fdt, -1, DT_SYSCFG_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read SYSCFG node in DT\n", __func__);
		return 0;
	}

	cuint = fdt_getprop(fdt, node, "reg", NULL);
	if (cuint == NULL) {
		return 0;
	}

	return fdt32_to_cpu(*cuint);
}

/*******************************************************************************
 * This function retrieves board model from DT
 * Returns string taken from model node, NULL otherwise
 ******************************************************************************/
const char *dt_get_board_model(void)
{
	int node = fdt_path_offset(fdt, "/");

	if (node < 0) {
		return NULL;
	}

	return (const char *)fdt_getprop(fdt, node, "model", NULL);
}
