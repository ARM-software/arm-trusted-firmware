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

#include <platform_def.h>
#include <plat_private.h>

static console_t console;

#if (defined(XILINX_OF_BOARD_DTB_ADDR) && !IS_TFA_IN_OCM(BL31_BASE))
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
 * Return: On success, it returns 1; on failure, it returns an 0.
 */
static uint32_t fdt_add_uart_info(dt_uart_info_t *info, int node, void *dtb)
{
	uintptr_t base_addr;
	const char *com;
	uint32_t ret = 0;

	com = fdt_getprop(dtb, node, "compatible", NULL);
	if (com != NULL) {
		strlcpy(info->compatible, com, sizeof(info->compatible));
	} else {
		ERROR("Compatible property not found in DTB node\n");
		ret  = -FDT_ERR_NOTFOUND;
		goto error;
	}

	ret = fdt_get_reg_props_by_index(dtb, node, 0, &base_addr, NULL);
	if (ret >= 0) {
		info->base = base_addr;
	} else {
		ERROR("Failed to retrieve base address. Error code: %d\n", ret);
		ret  = -FDT_ERR_NOTFOUND;
		goto error;
	}

	info->status = get_node_status(dtb, node);
	info->baud_rate = get_baudrate(dtb);

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
	int node, ret = 0;
	void *dtb = (void *)XILINX_OF_BOARD_DTB_ADDR;

	if (fdt_check_header(dtb) != 0) {
		ERROR("Can't read DT at %p\n", dtb);
		ret  = -FDT_ERR_NOTFOUND;
		goto error;
	}

	ret = fdt_open_into(dtb, dtb, XILINX_OF_BOARD_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, ret);
		ret  = -FDT_ERR_NOTFOUND;
		goto error;
	}

	node = fdt_get_stdout_node_offset(dtb);
	if (node < 0) {
		ERROR("DT get stdout node failed : %d\n", node);
		ret  = -FDT_ERR_NOTFOUND;
		goto error;
	}

	ret = fdt_add_uart_info(info, node, dtb);
	if (ret < 0) {
		ERROR("Failed to add DT UART info: %d\n", ret);
		ret  = -FDT_ERR_NOTFOUND;
		goto error;
	}

error:
	return ret;
}

/**
 * check_fdt_uart_info() - Check early uart info with DTB uart info.
 * @info: Pointer to the UART information structure.
 *
 * Return: On success, it returns 0; on failure, it returns an error+reason.
 */
static int check_fdt_uart_info(dt_uart_info_t *info)
{
	uint32_t ret = 0;

	if (info->status == 0) {
		ret = -ENODEV;
		goto error;
	}

	if ((info->base == console.base) &&
	   (info->baud_rate == UART_BAUDRATE) && !CONSOLE_IS(dcc)) {
		ret = -ENODEV;
		goto error;
	}

error:
	return ret;
}

/**
 * console_boot_end() - Unregister the console_t instance form the console list.
 * @boot_console: Pointer to the console information structure.
 */
static void console_boot_end(console_t *boot_console)
{
	if (CONSOLE_IS(dcc)) {
		console_dcc_unregister();
	} else {
		console_flush();
		(void)console_unregister(boot_console);
	}
}

/**
 * setup_runtime_console() - Registers the runtime uart with console list.
 * @clock: UART clock.
 * @info: Pointer to the UART information structure.
 */
static void setup_runtime_console(uint32_t clock, dt_uart_info_t *info)
{
	static console_t bl31_runtime_console;
	uint32_t rc;

#if defined(PLAT_zynqmp)
	rc = console_cdns_register(info->base,
				   clock,
				   info->baud_rate,
				   &bl31_runtime_console);
#else
	rc = console_pl011_register(info->base,
				    clock,
				    info->baud_rate,
				    &bl31_runtime_console);
#endif
	if (rc == 0) {
		panic();
	}

	console_set_scope(&bl31_runtime_console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME |
			  CONSOLE_FLAG_CRASH);
}


/**
 * runtime_console_init() - Initializes the run time console information.
 * @uart_info: Pointer to the UART information structure.
 * @bl31_boot_console: Pointer to the console information structure.
 * @clock: UART clock.
 *
 * Return: On success, it returns 0; on failure, it returns an error+reason;
 */
static int32_t runtime_console_init(dt_uart_info_t *uart_info,
			  console_t *bl31_boot_console,
			  uint32_t clock)
{
	int32_t rc = 0;

	/* Parse UART information from Device Tree Blob (DTB) */
	rc = fdt_get_uart_info(uart_info);
	if (rc < 0) {
		rc = -FDT_ERR_NOTFOUND;
	}

	if (strncmp(uart_info->compatible, DT_UART_COMPAT,
		   strlen(DT_UART_COMPAT)) == 0) {

		if (check_fdt_uart_info(uart_info) == 0) {
			setup_runtime_console(clock, uart_info);
			console_boot_end(bl31_boot_console);
			INFO("Runtime console setup\n");
		} else {
			INFO("Early console and DTB console are same\n");
		}
	} else if (strncmp(uart_info->compatible, DT_UART_DCC_COMPAT,
			  strlen(DT_UART_DCC_COMPAT)) == 0) {
		rc = console_dcc_register();
		if (rc == 0) {
			panic();
		}
		console_boot_end(bl31_boot_console);
	} else {
		WARN("BL31: No console device found in DT.\n");
	}

	return rc;
}
#endif

void setup_console(void)
{
	uint32_t rc;
	uint32_t uart_clk = get_uart_clk();

#if defined(PLAT_zynqmp)
	if (CONSOLE_IS(cadence) || (CONSOLE_IS(cadence1))) {
		rc = console_cdns_register(UART_BASE,
					   uart_clk,
					   UART_BAUDRATE,
					   &console);
		if (rc == 0) {
			panic();
		}

		console_set_scope(&console, CONSOLE_FLAG_BOOT |
				  CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
#else
	if (CONSOLE_IS(pl011) || (CONSOLE_IS(pl011_1))) {
		/* Initialize the console to provide early debug support */
		rc = console_pl011_register((uint32_t)UART_BASE,
					   uart_clk,
					   (uint32_t)UART_BAUDRATE,
					   &console);
		if (rc == 0) {
			panic();
		}

		console_set_scope(&console, CONSOLE_FLAG_BOOT |
				  CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}
#endif
	if (CONSOLE_IS(dcc)) {
		/* Initialize the dcc console for debug */
		rc = console_dcc_register();
		if (rc == 0) {
			panic();
		}
	}
	INFO("BL31: Early console setup\n");

#if (defined(XILINX_OF_BOARD_DTB_ADDR) && !IS_TFA_IN_OCM(BL31_BASE))
	static dt_uart_info_t uart_info = {0};

	/* Initialize the runtime console using UART information from the DTB */
	rc = runtime_console_init(&uart_info, &console, uart_clk);
	if (rc < 0) {
		ERROR("Failed to initialize runtime console: %d\n", rc);
	}
#endif
}
