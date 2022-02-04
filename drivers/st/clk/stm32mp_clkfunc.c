/*
 * Copyright (c) 2017-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>
#include <libfdt.h>

#include <platform_def.h>

#define DT_UART_COMPAT		"st,stm32h7-uart"
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

		if ((strncmp(cchar, name, (size_t)ret) == 0) &&
		    (fdt_get_status(subnode) != DT_DISABLED)) {
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
 * @param node_label: clock node name
 * @param prop_name: property name
 * @return: true/false regarding search result.
 */
bool fdt_clk_read_bool(const char *node_label, const char *prop_name)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
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

		if (strncmp(cchar, node_label, (size_t)ret) != 0) {
			continue;
		}

		if (fdt_getprop(fdt, subnode, prop_name, NULL) != NULL) {
			return true;
		}
	}

	return false;
}

/*
 * Get the value of a oscillator property from its name.
 * @param node_label: oscillator name
 * @param prop_name: property name
 * @param dflt_value: default value
 * @return oscillator value on success, default value if property not found.
 */
uint32_t fdt_clk_read_uint32_default(const char *node_label,
				     const char *prop_name, uint32_t dflt_value)
{
	int node, subnode;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
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

		if (strncmp(cchar, node_label, (size_t)ret) != 0) {
			continue;
		}

		return fdt_read_uint32_default(fdt, subnode, prop_name,
					       dflt_value);
	}

	return dflt_value;
}

/*
 * Get the RCC node offset from the device tree
 * @param fdt: Device tree reference
 * @return: Node offset or a negative value on error
 */
static int fdt_get_rcc_node(void *fdt)
{
	static int node;

	if (node <= 0) {
		node = fdt_node_offset_by_compatible(fdt, -1, DT_RCC_CLK_COMPAT);
	}

	return node;
}

/*
 * Read a series of parameters in rcc-clk section in device tree
 * @param prop_name: Name of the RCC property to be read
 * @param array: the array to store the property parameters
 * @param count: number of parameters to be read
 * @return: 0 on succes or a negative value on error
 */
int fdt_rcc_read_uint32_array(const char *prop_name, uint32_t count,
			      uint32_t *array)
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

	return fdt_read_uint32_array(fdt, node, prop_name, count, array);
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
 * Get the secure state for rcc node in device tree.
 * @return: true if rcc is configured for secure world access, false if not.
 */
bool fdt_get_rcc_secure_state(void)
{
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return false;
	}

	if (fdt_node_offset_by_compatible(fdt, -1, DT_RCC_SEC_CLK_COMPAT) < 0) {
		return false;
	}

	return true;
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

/*
 * Get the frequency of the specified UART instance.
 * @param instance: UART interface registers base address.
 * @return: clock frequency on success, 0 value on failure.
 */
unsigned long fdt_get_uart_clock_freq(uintptr_t instance)
{
	void *fdt;
	int node;
	int clk_id;

	if (fdt_get_address(&fdt) == 0) {
		return 0UL;
	}

	/* Check for UART nodes */
	node = dt_match_instance_by_compatible(DT_UART_COMPAT, instance);
	if (node < 0) {
		return 0UL;
	}

	clk_id = fdt_get_clock_id(node);
	if (clk_id < 0) {
		return 0UL;
	}

	return clk_get_rate((unsigned long)clk_id);
}

/*******************************************************************************
 * This function configures and restores the STGEN counter depending on the
 * connected clock.
 ******************************************************************************/
void stm32mp_stgen_config(unsigned long rate)
{
	uint32_t cntfid0;
	unsigned long long counter;

	cntfid0 = mmio_read_32(STGEN_BASE + CNTFID_OFF);

	if (cntfid0 == rate) {
		return;
	}

	mmio_clrbits_32(STGEN_BASE + CNTCR_OFF, CNTCR_EN);
	counter = stm32mp_stgen_get_counter() * rate / cntfid0;

	mmio_write_32(STGEN_BASE + CNTCVL_OFF, (uint32_t)counter);
	mmio_write_32(STGEN_BASE + CNTCVU_OFF, (uint32_t)(counter >> 32));
	mmio_write_32(STGEN_BASE + CNTFID_OFF, rate);
	mmio_setbits_32(STGEN_BASE + CNTCR_OFF, CNTCR_EN);

	write_cntfrq_el0(rate);

	/* Need to update timer with new frequency */
	generic_delay_timer_init();
}

/*******************************************************************************
 * This function returns the STGEN counter value.
 ******************************************************************************/
unsigned long long stm32mp_stgen_get_counter(void)
{
	return (((unsigned long long)mmio_read_32(STGEN_BASE + CNTCVU_OFF) << 32) |
		mmio_read_32(STGEN_BASE + CNTCVL_OFF));
}

/*******************************************************************************
 * This function restores the STGEN counter value.
 * It takes a first input value as a counter backup value to be restored and a
 * offset in ms to be added.
 ******************************************************************************/
void stm32mp_stgen_restore_counter(unsigned long long value,
				   unsigned long long offset_in_ms)
{
	unsigned long long cnt;

	cnt = value + ((offset_in_ms *
			mmio_read_32(STGEN_BASE + CNTFID_OFF)) / 1000U);

	mmio_clrbits_32(STGEN_BASE + CNTCR_OFF, CNTCR_EN);
	mmio_write_32(STGEN_BASE + CNTCVL_OFF, (uint32_t)cnt);
	mmio_write_32(STGEN_BASE + CNTCVU_OFF, (uint32_t)(cnt >> 32));
	mmio_setbits_32(STGEN_BASE + CNTCR_OFF, CNTCR_EN);
}
