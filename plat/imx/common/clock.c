/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <mmio.h>
#include <stdint.h>
#include <stdbool.h>
#include <imx-regs.h>
#include "clock.h"

static void mxc_clock_target_set(unsigned int id, uint32_t val)
{
	struct ccm *ccm = ((struct ccm *)CCM_BASE);
	uintptr_t addr;

	if (id > CCM_ROOT_CTRL_NUM)
		return;

	addr = (uintptr_t)&ccm->ccm_root_ctrl[id].ccm_target_root;
	mmio_write_32(addr, val);
}

static void mxc_clock_target_clr(unsigned int id, uint32_t val)
{
	struct ccm *ccm = ((struct ccm *)CCM_BASE);
	uintptr_t addr;

	if (id > CCM_ROOT_CTRL_NUM)
		return;

	addr = (uintptr_t)&ccm->ccm_root_ctrl[id].ccm_target_root_clr;
	mmio_write_32(addr, val);
}

static void mxc_clock_gate_enable(unsigned int id, bool enable)
{
	struct ccm *ccm = ((struct ccm *)CCM_BASE);
	uintptr_t addr;

	if (id > CCM_CLK_GATE_CTRL_NUM)
		return;

	/* TODO: add support for more than DOMAIN0 clocks */
	if (enable)
		addr = (uintptr_t)&ccm->ccm_clk_gate_ctrl[id].ccm_ccgr_set;
	else
		addr = (uintptr_t)&ccm->ccm_clk_gate_ctrl[id].ccm_ccgr_clr;

	mmio_write_32(addr, CCM_CCGR_SETTING0_DOM_CLK_ALWAYS);
}

void clock_enable_uart(unsigned int uart_id, uint32_t uart_clk_en_bits)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_UART1_CLK_ROOT + uart_id;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_UART1 + uart_id;

	/* Check for error */
	if (uart_id > MXC_MAX_UART_NUM)
		return;

	/* Set target register values */
	mxc_clock_target_set(ccm_trgt_id, uart_clk_en_bits);

	/* Enable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_id, true);
}

void clock_disable_uart(unsigned int uart_id)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_UART1_CLK_ROOT + uart_id;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_UART1 + uart_id;

	/* Check for error */
	if (uart_id > MXC_MAX_UART_NUM)
		return;

	/* Disable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_id, false);

	/* Clear the target */
	mxc_clock_target_clr(ccm_trgt_id, 0xFFFFFFFF);
}

void clock_enable_usdhc(unsigned int usdhc_id, uint32_t usdhc_clk_en_bits)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_USDHC1_CLK_ROOT + usdhc_id;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_USBHDC1 + usdhc_id;

	/* Check for error */
	if (usdhc_id > MXC_MAX_USDHC_NUM)
		return;

	/* Set target register values */
	mxc_clock_target_set(ccm_trgt_id, usdhc_clk_en_bits);

	/* Enable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_id, true);
}

static void clock_uart_init(void)
{
	unsigned int i;

	for (i = 0; i < MXC_MAX_UART_NUM; i++)
		clock_disable_uart(i);
}

void clock_enable_wdog(unsigned int wdog_id)
{
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_WDOG1 + wdog_id;

	/* Check for error */
	if (wdog_id > MXC_MAX_WDOG_NUM)
		return;

	/* Enable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_id, true);
}

void clock_disable_wdog(unsigned int wdog_id)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_WDOG_CLK_ROOT;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_WDOG1 + wdog_id;

	/* Check for error */
	if (wdog_id > MXC_MAX_WDOG_NUM)
		return;

	/* Disable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_id, false);

	/* Clear the target */
	mxc_clock_target_clr(ccm_trgt_id, 0xFFFFFFFF);
}

void clock_set_wdog_clk_root_bits(uint32_t wdog_clk_root_en_bits)
{
	/* Enable the common clock root just once */
	mxc_clock_target_set(CCM_TRT_ID_WDOG_CLK_ROOT, wdog_clk_root_en_bits);
}

static void clock_wdog_init(void)
{
	unsigned int i;

	for (i = 0; i < MXC_MAX_WDOG_NUM; i++)
		clock_disable_wdog(i);
}

void clock_enable_usb(unsigned int ccm_ccgr_usb_id)
{
	/* Enable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_usb_id, true);
}

void clock_disable_usb(unsigned int ccm_ccgr_usb_id)
{
	/* Disable the clock gate */
	mxc_clock_gate_enable(ccm_ccgr_usb_id, false);
}

void clock_set_usb_clk_root_bits(uint32_t usb_clk_root_en_bits)
{
	/* Enable the common clock root just once */
	mxc_clock_target_set(CCM_TRT_ID_USB_HSIC_CLK_ROOT, usb_clk_root_en_bits);
}

static void clock_usb_init(void)
{
	/* Disable the clock root */
	mxc_clock_target_clr(CCM_TRT_ID_USB_HSIC_CLK_ROOT, 0xFFFFFFFF);
}

void clock_init(void)
{
	/*
	 * The BootROM hands off to the next stage with the internal 24 MHz XTAL
	 * crystal already clocking the main PLL, which is very handy.
	 * Here we should enable whichever peripherals are required for ATF and
	 * OPTEE.
	 *
	 * Subsequent stages in the boot process such as u-boot and Linux
	 * already have a significant and mature code-base around clocks, so our
	 * objective should be to enable what we need for ATF/OPTEE without
	 * breaking any existing upstream code in Linux and u-boot.
	 */

	/* Initialize UART clocks */
	clock_uart_init();

	/* Watchdog clocks */
	clock_wdog_init();

	/* USB clocks */
	clock_usb_init();

}
