/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp1_clkfunc.h>
#include <drivers/st/stm32mp1_ddr.h>
#include <drivers/st/stm32mp1_ram.h>

#include <stm32mp1_dt.h>

#define DT_GPIO_BANK_SHIFT	12
#define DT_GPIO_BANK_MASK	0x1F000U
#define DT_GPIO_PIN_SHIFT	8
#define DT_GPIO_PIN_MASK	0xF00U
#define DT_GPIO_MODE_MASK	0xFFU

static int fdt_checked;

static void *fdt = (void *)(uintptr_t)STM32MP1_DTB_BASE;

/*******************************************************************************
 * This function gets the pin settings from DT information.
 * When analyze and parsing is done, set the GPIO registers.
 * Return 0 on success, else return a negative FDT_ERR_xxx error code.
 ******************************************************************************/
static int dt_set_gpio_config(int node)
{
	const fdt32_t *cuint, *slewrate;
	int len, pinctrl_node, pinctrl_subnode;
	uint32_t i;
	uint32_t speed = GPIO_SPEED_LOW;
	uint32_t pull = GPIO_NO_PULL;

	cuint = fdt_getprop(fdt, node, "pinmux", &len);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	pinctrl_node = fdt_parent_offset(fdt, fdt_parent_offset(fdt, node));
	if (pinctrl_node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	slewrate = fdt_getprop(fdt, node, "slew-rate", NULL);
	if (slewrate != NULL) {
		speed = fdt32_to_cpu(*slewrate);
	}

	if (fdt_getprop(fdt, node, "bias-pull-up", NULL) != NULL) {
		pull = GPIO_PULL_UP;
	} else if (fdt_getprop(fdt, node, "bias-pull-down", NULL) != NULL) {
		pull = GPIO_PULL_DOWN;
	} else {
		VERBOSE("No bias configured in node %d\n", node);
	}

	for (i = 0; i < ((uint32_t)len / sizeof(uint32_t)); i++) {
		uint32_t pincfg;
		uint32_t bank;
		uint32_t pin;
		uint32_t mode;
		uint32_t alternate = GPIO_ALTERNATE_0;

		pincfg = fdt32_to_cpu(*cuint);
		cuint++;

		bank = (pincfg & DT_GPIO_BANK_MASK) >> DT_GPIO_BANK_SHIFT;

		pin = (pincfg & DT_GPIO_PIN_MASK) >> DT_GPIO_PIN_SHIFT;

		mode = pincfg & DT_GPIO_MODE_MASK;

		switch (mode) {
		case 0:
			mode = GPIO_MODE_INPUT;
			break;
		case 1 ... 16:
			alternate = mode - 1U;
			mode = GPIO_MODE_ALTERNATE;
			break;
		case 17:
			mode = GPIO_MODE_ANALOG;
			break;
		default:
			mode = GPIO_MODE_OUTPUT;
			break;
		}

		if (fdt_getprop(fdt, node, "drive-open-drain", NULL) != NULL) {
			mode |= GPIO_OPEN_DRAIN;
		}

		fdt_for_each_subnode(pinctrl_subnode, fdt, pinctrl_node) {
			uint32_t bank_offset;
			const fdt32_t *cuint2;

			if (fdt_getprop(fdt, pinctrl_subnode,
					"gpio-controller", NULL) == NULL) {
				continue;
			}

			cuint2 = fdt_getprop(fdt, pinctrl_subnode, "reg", NULL);
			if (cuint2 == NULL) {
				continue;
			}

			if (bank == GPIO_BANK_Z) {
				bank_offset = 0;
			} else {
				bank_offset = bank * STM32_GPIO_BANK_OFFSET;
			}

			if (fdt32_to_cpu(*cuint2) == bank_offset) {
				int clk_id = fdt_get_clock_id(pinctrl_subnode);

				if (clk_id < 0) {
					return -FDT_ERR_NOTFOUND;
				}

				if (stm32mp1_clk_enable((unsigned long)clk_id) <
				    0) {
					return -FDT_ERR_BADVALUE;
				}

				break;
			}
		}

		set_gpio(bank, pin, mode, speed, pull, alternate);
	}

	return 0;
}

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

/*******************************************************************************
 * This function gets the pin settings from DT information.
 * When analyze and parsing is done, set the GPIO registers.
 * Returns 0 if success, and a negative value else.
 ******************************************************************************/
int dt_set_pinctrl_config(int node)
{
	const fdt32_t *cuint;
	int lenp = 0;
	uint32_t i;

	if (!fdt_check_status(node)) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint = fdt_getprop(fdt, node, "pinctrl-0", &lenp);
	if (cuint == NULL) {
		return -FDT_ERR_NOTFOUND;
	}

	for (i = 0; i < ((uint32_t)lenp / 4U); i++) {
		int phandle_node, phandle_subnode;

		phandle_node =
			fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
		if (phandle_node < 0) {
			return -FDT_ERR_NOTFOUND;
		}

		fdt_for_each_subnode(phandle_subnode, fdt, phandle_node) {
			int ret = dt_set_gpio_config(phandle_subnode);

			if (ret < 0) {
				return ret;
			}
		}

		cuint++;
	}

	return 0;
}

/*******************************************************************************
 * This function gets the stdout pin configuration information from the DT.
 * And then calls the sub-function to treat it and set GPIO registers.
 * Returns 0 if success, and a negative value else.
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

	info->status = fdt_check_status(node);
	info->sec_status = fdt_check_secure_status(node);
}

/*******************************************************************************
 * This function retrieve the generic information from DT.
 * Returns node if success, and a negative value else.
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
 * Returns node if success, and a negative value else.
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
 * This function gets the stdout path node.
 * It reads the value indicated inside the device tree.
 * Returns node if success, and a negative value else.
 ******************************************************************************/
int dt_get_stdout_node_offset(void)
{
	int node;
	const char *cchar;

	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
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
 * This function gets DDR size information from the DT.
 * Returns value in bytes if success, and STM32MP1_DDR_SIZE_DFLT else.
 ******************************************************************************/
uint32_t dt_get_ddr_size(void)
{
	int node;

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read DDR node in DT\n", __func__);
		return STM32MP1_DDR_SIZE_DFLT;
	}

	return fdt_read_uint32_default(node, "st,mem-size",
				       STM32MP1_DDR_SIZE_DFLT);
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
