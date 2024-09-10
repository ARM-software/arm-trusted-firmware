/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/dcc.h>
#include <drivers/arm/pl011.h>
#include <drivers/cadence/cdns_uart.h>
#include <drivers/console.h>
#include <libfdt.h>
#include <plat_console.h>
#include <plat_fdt.h>

#include <platform_def.h>
#include <plat_private.h>

#if !(CONSOLE_IS(none))
static console_t boot_console;
static console_holder boot_hd_console;
#if defined(CONSOLE_RUNTIME)
static console_t runtime_console;
static console_holder rt_hd_console;
#endif

#if ((CONSOLE_IS(dtb) || RT_CONSOLE_IS(dtb)) && defined(XILINX_OF_BOARD_DTB_ADDR)) && \
	(!defined(PLAT_zynqmp) || (defined(PLAT_zynqmp) && \
				   !IS_TFA_IN_OCM(BL31_BASE)))
static dt_uart_info_t dt_uart_info;
#endif

/**
 * register_console() - Registers the uart with console list.
 * @consoleh: Console holder structure with UART base address,
 *  UART clock, UART buad rate, flags & console type
 * @console: Pointer to the console information structure.
 */
static void register_console(const console_holder *consoleh, console_t *console)
{
	int32_t rc = 0;

	switch (consoleh->console_type) {
#if defined(PLAT_zynqmp)
	case CONSOLE_CDNS:
		rc = console_cdns_register(consoleh->base,
				consoleh->clk,
				consoleh->baud_rate,
				console);
		break;
#else
	case CONSOLE_PL011:
		rc = console_pl011_register(consoleh->base,
				consoleh->clk,
				consoleh->baud_rate,
				console);
		break;
#endif
	case CONSOLE_DCC:
		rc = console_dcc_register(console);
		break;
	default:
		INFO("Invalid console type\n");
		break;
	}

	if (rc == 0) {
		panic();
	}

	console_set_scope(console, consoleh->console_scope);
}

#if ((CONSOLE_IS(dtb) || RT_CONSOLE_IS(dtb)) && defined(XILINX_OF_BOARD_DTB_ADDR)) && \
	(!defined(PLAT_zynqmp) || (defined(PLAT_zynqmp) && \
				   !IS_TFA_IN_OCM(BL31_BASE)))
/**
 * get_baudrate() - Get the baudrate form DTB.
 * @dtb: Address of the Device Tree Blob (DTB).
 *
 * Return: On success returns the baudrate; on failure returns an error.
 */
static int32_t get_baudrate(void *dtb)
{
	int node;
	int32_t ret = 0;
	const char *prop, *path;
	char *end;
	int32_t baud_rate = 0;

	node = fdt_path_offset(dtb, "/secure-chosen");
	if (node < 0) {
		node = fdt_path_offset(dtb, "/chosen");
		if (node < 0) {
			ret = -FDT_ERR_NOTFOUND;
			goto error;
		}
	}

	prop = fdt_getprop(dtb, node, "stdout-path", NULL);
	if (prop == NULL) {
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	}

	/* Parse string serial0:115200n8 */
	path = strchr(prop, ':');
	if (!path) {
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	} else {

		baud_rate = strtoul(path + 1, &end, 10);
		if (baud_rate == 0 && end == path) {
			ERROR("Conversion error occurred: %d\n", baud_rate);
			ret = -FDT_ERR_NOTFOUND;
			goto error;
		}
		ret = baud_rate;
	}

error:
	return ret;
}

/**
 * get_node_status() - Get the DTB node status.
 * @dtb: Address of the Device Tree Blob (DTB).
 * @node: Node address in the device tree.
 *
 * Return: On success, it returns 1; on failure, it returns an 0.
 */
static uint32_t get_node_status(void *dtb, int node)
{
	const char *status_cell;
	uint32_t status = 0;

	status_cell = fdt_getprop(dtb, node, "status", NULL);
	if (!status_cell || strcmp(status_cell, "okay") == 0) {
		status = 1;
	} else {
		status = 0;
	}

	return status;
}

/**
 * fdt_add_uart_info() - Add DTB information to a UART structure.
 * @info: Pointer to the UART information structure.
 * @node: Node address in the device tree.
 * @dtb: Address of the Device Tree Blob(DTB).
 *
 * Return: On success, it returns 0; on failure, it returns -1 or -FDT_ERR_NOTFOUND.
 */
static int32_t fdt_add_uart_info(dt_uart_info_t *info, int node, void *dtb)
{
	uintptr_t base_addr;
	const char *com;
	int32_t ret = 0;
	uint32_t status;

	com = fdt_getprop(dtb, node, "compatible", NULL);
	if (com != NULL) {
		strlcpy(info->compatible, com, sizeof(info->compatible));
	} else {
		ERROR("Compatible property not found in DTB node\n");
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	}

	status = get_node_status(dtb, node);
	if (status == 0) {
		ERROR("Uart node is disabled in DTB\n");
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	}

	if (strncmp(info->compatible, DT_UART_DCC_COMPAT, strlen(DT_UART_DCC_COMPAT)) != 0) {
		ret = fdt_get_reg_props_by_index(dtb, node, 0, &base_addr, NULL);
		if (ret >= 0) {
			info->base = base_addr;
		} else {
			ERROR("Failed to retrieve base address. Error code: %d\n", ret);
			ret = -FDT_ERR_NOTFOUND;
			goto error;
		}

		info->baud_rate = get_baudrate(dtb);

		if (strncmp(info->compatible, DT_UART_CAD_COMPAT,
					strlen(DT_UART_CAD_COMPAT)) == 0) {
			info->console_type = CONSOLE_CDNS;
		} else if (strncmp(info->compatible, DT_UART_PL011_COMPAT,
					strlen(DT_UART_PL011_COMPAT)) == 0) {
			info->console_type = CONSOLE_PL011;
		} else {
			ERROR("Incompatible uart node in DTB\n");
			ret = -FDT_ERR_NOTFOUND;
		}
	} else {
		info->console_type = CONSOLE_DCC;
	}

error:
	return ret;
}

/**
 * fdt_get_uart_info() - Get the uart information form DTB.
 * @info: Pointer to the UART information structure.
 *
 * Return: On success, it returns 0; on failure, it returns an error+reason.
 */
static int fdt_get_uart_info(dt_uart_info_t *info)
{
	int node = 0, ret = 0;
	void *dtb = (void *)XILINX_OF_BOARD_DTB_ADDR;

	ret = is_valid_dtb(dtb);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, ret);
		goto error;
	}

	node = fdt_get_stdout_node_offset(dtb);
	if (node < 0) {
		ERROR("DT get stdout node failed : %d\n", node);
		goto error;
	}

	ret = fdt_add_uart_info(info, node, dtb);
	if (ret < 0) {
		ERROR("Failed to add DT UART info: %d\n", ret);
		goto error;
	}

error:
	return ret;
}
#endif

void setup_console(void)
{
	/* This is hardcoded console setup just in case that DTB console fails */
	boot_hd_console.base = (uintptr_t)UART_BASE;
	boot_hd_console.baud_rate = (uint32_t)UART_BAUDRATE;
	boot_hd_console.clk = get_uart_clk();
	boot_hd_console.console_scope = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH;
	boot_hd_console.console_type = UART_TYPE;

	/* For DT code decoding uncomment console registration below */
	/* register_console(&boot_hd_console, &boot_console); */

#if ((CONSOLE_IS(dtb) || RT_CONSOLE_IS(dtb)) && defined(XILINX_OF_BOARD_DTB_ADDR)) && \
	(!defined(PLAT_zynqmp) || (defined(PLAT_zynqmp) && \
				   !IS_TFA_IN_OCM(BL31_BASE)))
	/* Parse DTB console for UART information  */
	if (fdt_get_uart_info(&dt_uart_info) == 0) {
		if (CONSOLE_IS(dtb)) {
			boot_hd_console.base = dt_uart_info.base;
			boot_hd_console.baud_rate = dt_uart_info.baud_rate;
			boot_hd_console.console_type = dt_uart_info.console_type;
		}
	} else {
		ERROR("Failed to initialize DT console or console node is disabled\n");
	}
#endif

	/* Initialize the boot console */
	register_console(&boot_hd_console, &boot_console);

	INFO("BL31: Early console setup\n");

#ifdef CONSOLE_RUNTIME
#if (RT_CONSOLE_IS(dtb) && defined(XILINX_OF_BOARD_DTB_ADDR)) && \
	       (!defined(PLAT_zynqmp) || (defined(PLAT_zynqmp) && \
					!IS_TFA_IN_OCM(BL31_BASE)))
	rt_hd_console.base = dt_uart_info.base;
	rt_hd_console.baud_rate = dt_uart_info.baud_rate;
	rt_hd_console.console_type = dt_uart_info.console_type;
#else
	rt_hd_console.base = (uintptr_t)RT_UART_BASE;
	rt_hd_console.baud_rate = (uint32_t)UART_BAUDRATE;
	rt_hd_console.console_type = RT_UART_TYPE;
#endif

	if ((rt_hd_console.console_type == boot_hd_console.console_type) &&
			(rt_hd_console.base == boot_hd_console.base)) {
		console_set_scope(&boot_console,
				CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH | CONSOLE_FLAG_RUNTIME);
		INFO("Successfully initialized runtime console\n");
	} else {
		rt_hd_console.clk = get_uart_clk();
		rt_hd_console.console_scope = CONSOLE_FLAG_RUNTIME;

		register_console(&rt_hd_console, &runtime_console);
		INFO("Successfully initialized new runtime console\n");
	}
#endif
}
#else
void setup_console(void)
{
}
#endif
