/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <lib/bl_aux_params/bl_aux_params.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/gpio.h>
#include <libfdt.h>
#include <lib/coreboot.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <plat_params.h>
#include <plat_private.h>

static struct bl_aux_gpio_info rst_gpio;
static struct bl_aux_gpio_info poweroff_gpio;
static struct bl_aux_gpio_info suspend_gpio[10];
uint32_t suspend_gpio_cnt;
static struct bl_aux_rk_apio_info suspend_apio;
static uint32_t rk_uart_base = PLAT_RK_UART_BASE;

uint32_t rockchip_get_uart_base(void)
{
	return rk_uart_base;
}

#if COREBOOT
static int dt_process_fdt(u_register_t param_from_bl2)
{
	return -ENODEV;
}
#else
static uint8_t fdt_buffer[0x10000];

void *plat_get_fdt(void)
{
	return &fdt_buffer[0];
}

static void plat_rockchip_dt_process_fdt_uart(void *fdt)
{
	const char *path_name = "/chosen";
	const char *prop_name = "stdout-path";
	int node_offset;
	int stdout_path_len;
	const char *stdout_path;
	char serial_char;
	int serial_no;
	uint32_t uart_base;

	node_offset = fdt_path_offset(fdt, path_name);
	if (node_offset < 0)
		return;

	stdout_path = fdt_getprop(fdt, node_offset, prop_name,
				  &stdout_path_len);
	if (stdout_path == NULL)
		return;

	/*
	 * We expect something like:
	 *   "serial0:...""
	 */
	if (strncmp("serial", stdout_path, 6) != 0)
		return;

	serial_char = stdout_path[6];
	serial_no = serial_char - '0';

	switch (serial_no) {
	case 0:
		uart_base = UART0_BASE;
		break;
	case 1:
		uart_base = UART1_BASE;
		break;
	case 2:
		uart_base = UART2_BASE;
		break;
#ifdef UART3_BASE
	case 3:
		uart_base = UART3_BASE;
		break;
#endif
#ifdef UART4_BASE
	case 4:
		uart_base = UART4_BASE;
		break;
#endif
	default:
		return;
	}

	rk_uart_base = uart_base;
}

static int dt_process_fdt(u_register_t param_from_bl2)
{
	void *fdt = plat_get_fdt();
	int ret;

	ret = fdt_open_into((void *)param_from_bl2, fdt, 0x10000);
	if (ret < 0)
		return ret;

	plat_rockchip_dt_process_fdt_uart(fdt);

	return 0;
}
#endif

struct bl_aux_gpio_info *plat_get_rockchip_gpio_reset(void)
{
	return &rst_gpio;
}

struct bl_aux_gpio_info *plat_get_rockchip_gpio_poweroff(void)
{
	return &poweroff_gpio;
}

struct bl_aux_gpio_info *plat_get_rockchip_suspend_gpio(uint32_t *count)
{
	*count = suspend_gpio_cnt;

	return &suspend_gpio[0];
}

struct bl_aux_rk_apio_info *plat_get_rockchip_suspend_apio(void)
{
	return &suspend_apio;
}

static bool rk_aux_param_handler(struct bl_aux_param_header *param)
{
	/* Store platform parameters for later processing if needed. */
	switch (param->type) {
	case BL_AUX_PARAM_RK_RESET_GPIO:
		rst_gpio = ((struct bl_aux_param_gpio *)param)->gpio;
		return true;
	case BL_AUX_PARAM_RK_POWEROFF_GPIO:
		poweroff_gpio = ((struct bl_aux_param_gpio *)param)->gpio;
		return true;
	case BL_AUX_PARAM_RK_SUSPEND_GPIO:
		if (suspend_gpio_cnt >= ARRAY_SIZE(suspend_gpio)) {
			ERROR("Exceeded the supported suspend GPIO number.\n");
			return true;
		}
		suspend_gpio[suspend_gpio_cnt++] =
			((struct bl_aux_param_gpio *)param)->gpio;
		return true;
	case BL_AUX_PARAM_RK_SUSPEND_APIO:
		suspend_apio = ((struct bl_aux_param_rk_apio *)param)->apio;
		return true;
	}

	return false;
}

void params_early_setup(u_register_t plat_param_from_bl2)
{
	/*
	 * Test if this is a FDT passed as a platform-specific parameter
	 * block.
	 */
	if (!dt_process_fdt(plat_param_from_bl2))
		return;

	bl_aux_params_parse(plat_param_from_bl2, rk_aux_param_handler);
}
