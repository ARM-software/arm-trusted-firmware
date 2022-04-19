/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/st/regulator.h>
#include <drivers/st/stm32_gpio.h>
#include <libfdt.h>

#include <platform_def.h>
#include <stm32mp_dt.h>

static void *fdt;

/*******************************************************************************
 * This function checks device tree file with its header.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_open_and_check(uintptr_t dt_addr)
{
	int ret;

	ret = fdt_check_header((void *)dt_addr);
	if (ret == 0) {
		fdt = (void *)dt_addr;
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
	if (fdt == NULL) {
		return 0;
	}

	*fdt_addr = fdt;

	return 1;
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
	const char *cchar;

	cchar = fdt_getprop(fdt, node, "status", NULL);
	if ((cchar == NULL) ||
	    (strncmp(cchar, "okay", strlen("okay")) == 0)) {
		status |= DT_NON_SECURE;
	}

	cchar = fdt_getprop(fdt, node, "secure-status", NULL);
	if (cchar == NULL) {
		if (status == DT_NON_SECURE) {
			status |= DT_SECURE;
		}
	} else if (strncmp(cchar, "okay", strlen("okay")) == 0) {
		status |= DT_SECURE;
	}

	return status;
}

#if ENABLE_ASSERTIONS
/*******************************************************************************
 * This function returns the address cells from the node parent.
 * Returns:
 * - #address-cells value if success.
 * - invalid value if error.
 * - a default value if undefined #address-cells property as per libfdt
 *   implementation.
 ******************************************************************************/
static int fdt_get_node_parent_address_cells(int node)
{
	int parent;

	parent = fdt_parent_offset(fdt, node);
	if (parent < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return fdt_address_cells(fdt, parent);
}
#endif

/*******************************************************************************
 * This function gets the stdout pin configuration information from the DT.
 * And then calls the sub-function to treat it and set GPIO registers.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int dt_set_stdout_pinctrl(void)
{
	int node;

	node = fdt_get_stdout_node_offset(fdt);
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

	assert(fdt_get_node_parent_address_cells(node) == 1);

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

	node = fdt_get_stdout_node_offset(fdt);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	dt_fill_device_info(info, node);

	return node;
}

/*******************************************************************************
 * This function returns the node offset matching compatible string in the DT,
 * and also matching the reg property with the given address.
 * Returns value on success, and error value on failure.
 ******************************************************************************/
int dt_match_instance_by_compatible(const char *compatible, uintptr_t address)
{
	int node;

	fdt_for_each_compatible_node(fdt, node, compatible) {
		const fdt32_t *cuint;

		assert(fdt_get_node_parent_address_cells(node) == 1);

		cuint = fdt_getprop(fdt, node, "reg", NULL);
		if (cuint == NULL) {
			continue;
		}

		if ((uintptr_t)fdt32_to_cpu(*cuint) == address) {
			return node;
		}
	}

	return -FDT_ERR_NOTFOUND;
}

/*******************************************************************************
 * This function gets DDR size information from the DT.
 * Returns value in bytes on success, and 0 on failure.
 ******************************************************************************/
uint32_t dt_get_ddr_size(void)
{
	static uint32_t size;
	int node;

	if (size != 0U) {
		return size;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		INFO("%s: Cannot read DDR node in DT\n", __func__);
		return 0;
	}

	size = fdt_read_uint32_default(fdt, node, "st,mem-size", 0U);

	flush_dcache_range((uintptr_t)&size, sizeof(uint32_t));

	return size;
}

/*******************************************************************************
 * This function gets PWR VDD regulator voltage information from the DT.
 * Returns value in microvolts on success, and 0 on failure.
 ******************************************************************************/
uint32_t dt_get_pwr_vdd_voltage(void)
{
	struct rdev *regul = dt_get_vdd_regulator();
	uint16_t min;

	if (regul == NULL) {
		return 0;
	}

	regulator_get_range(regul, &min, NULL);

	return (uint32_t)min * 1000U;
}

/*******************************************************************************
 * This function retrieves VDD supply regulator from DT.
 * Returns an rdev taken from supply node, NULL otherwise.
 ******************************************************************************/
struct rdev *dt_get_vdd_regulator(void)
{
	int node = fdt_node_offset_by_compatible(fdt, -1, DT_PWR_COMPAT);

	if (node < 0) {
		return NULL;
	}

	return regulator_get_by_supply_name(fdt, node, "vdd");
}

/*******************************************************************************
 * This function retrieves CPU supply regulator from DT.
 * Returns an rdev taken from supply node, NULL otherwise.
 ******************************************************************************/
struct rdev *dt_get_cpu_regulator(void)
{
	int node = fdt_path_offset(fdt, "/cpus/cpu@0");

	if (node < 0) {
		return NULL;
	}

	return regulator_get_by_supply_name(fdt, node, "cpu");
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

/*******************************************************************************
 * dt_find_otp_name: get OTP ID and length in DT.
 * name: sub-node name to look up.
 * otp: pointer to read OTP number or NULL.
 * otp_len: pointer to read OTP length in bits or NULL.
 * return value: 0 if no error, an FDT error value otherwise.
 ******************************************************************************/
int dt_find_otp_name(const char *name, uint32_t *otp, uint32_t *otp_len)
{
	int node;
	int len;
	const fdt32_t *cuint;

	if ((name == NULL) || (otp == NULL)) {
		return -FDT_ERR_BADVALUE;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_BSEC_COMPAT);
	if (node < 0) {
		return node;
	}

	node = fdt_subnode_offset(fdt, node, name);
	if (node < 0) {
		ERROR("nvmem node %s not found\n", name);
		return node;
	}

	cuint = fdt_getprop(fdt, node, "reg", &len);
	if ((cuint == NULL) || (len != (2 * (int)sizeof(uint32_t)))) {
		ERROR("Malformed nvmem node %s: ignored\n", name);
		return -FDT_ERR_BADVALUE;
	}

	if (fdt32_to_cpu(*cuint) % sizeof(uint32_t)) {
		ERROR("Misaligned nvmem %s element: ignored\n", name);
		return -FDT_ERR_BADVALUE;
	}

	if (otp != NULL) {
		*otp = fdt32_to_cpu(*cuint) / sizeof(uint32_t);
	}

	if (otp_len != NULL) {
		cuint++;
		*otp_len = fdt32_to_cpu(*cuint) * CHAR_BIT;
	}

	return 0;
}

/*******************************************************************************
 * This function gets the pin count for a GPIO bank based from the FDT.
 * It also checks node consistency.
 ******************************************************************************/
int fdt_get_gpio_bank_pin_count(unsigned int bank)
{
	int pinctrl_node;
	int node;
	uint32_t bank_offset;

	pinctrl_node = stm32_get_gpio_bank_pinctrl_node(fdt, bank);
	if (pinctrl_node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	bank_offset = stm32_get_gpio_bank_offset(bank);

	fdt_for_each_subnode(node, fdt, pinctrl_node) {
		const fdt32_t *cuint;
		int pin_count;
		int len;
		int i;

		if (fdt_getprop(fdt, node, "gpio-controller", NULL) == NULL) {
			continue;
		}

		cuint = fdt_getprop(fdt, node, "reg", NULL);
		if (cuint == NULL) {
			continue;
		}

		if (fdt32_to_cpu(*cuint) != bank_offset) {
			continue;
		}

		if (fdt_get_status(node) == DT_DISABLED) {
			return 0;
		}

		/* Parse gpio-ranges with its 4 parameters */
		cuint = fdt_getprop(fdt, node, "gpio-ranges", &len);
		len /= sizeof(*cuint);
		if ((len % 4) != 0) {
			return -FDT_ERR_BADVALUE;
		}

		/* Get the last defined gpio line (offset + nb of pins) */
		pin_count = fdt32_to_cpu(*(cuint + 1)) + fdt32_to_cpu(*(cuint + 3));
		for (i = 0; i < len / 4; i++) {
			pin_count = MAX(pin_count, (int)(fdt32_to_cpu(*(cuint + 1)) +
							 fdt32_to_cpu(*(cuint + 3))));
			cuint += 4;
		}

		return pin_count;
	}

	return 0;
}
