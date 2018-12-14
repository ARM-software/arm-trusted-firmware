/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>

#include <arch.h>
#include <lib/mmio.h>

#include <imx_regs.h>
#include <imx_clock.h>

void imx_clock_target_set(unsigned int id, uint32_t val)
{
	struct ccm *ccm = ((struct ccm *)CCM_BASE);
	uintptr_t addr;

	if (id > CCM_ROOT_CTRL_NUM)
		return;

	addr = (uintptr_t)&ccm->ccm_root_ctrl[id].ccm_target_root;
	mmio_write_32(addr, val);
}

void imx_clock_target_clr(unsigned int id, uint32_t val)
{
	struct ccm *ccm = ((struct ccm *)CCM_BASE);
	uintptr_t addr;

	if (id > CCM_ROOT_CTRL_NUM)
		return;

	addr = (uintptr_t)&ccm->ccm_root_ctrl[id].ccm_target_root_clr;
	mmio_write_32(addr, val);
}

void imx_clock_gate_enable(unsigned int id, bool enable)
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

void imx_clock_enable_uart(unsigned int uart_id, uint32_t uart_clk_en_bits)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_UART1_CLK_ROOT + uart_id;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_UART1 + uart_id;

	/* Check for error */
	if (uart_id > MXC_MAX_UART_NUM)
		return;

	/* Set target register values */
	imx_clock_target_set(ccm_trgt_id, uart_clk_en_bits);

	/* Enable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_id, true);
}

void imx_clock_disable_uart(unsigned int uart_id)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_UART1_CLK_ROOT + uart_id;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_UART1 + uart_id;

	/* Check for error */
	if (uart_id > MXC_MAX_UART_NUM)
		return;

	/* Disable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_id, false);

	/* Clear the target */
	imx_clock_target_clr(ccm_trgt_id, 0xFFFFFFFF);
}

void imx_clock_enable_usdhc(unsigned int usdhc_id, uint32_t usdhc_clk_en_bits)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_USDHC1_CLK_ROOT + usdhc_id;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_USBHDC1 + usdhc_id;

	/* Check for error */
	if (usdhc_id > MXC_MAX_USDHC_NUM)
		return;

	/* Set target register values */
	imx_clock_target_set(ccm_trgt_id, usdhc_clk_en_bits);

	/* Enable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_id, true);
}

void imx_clock_enable_wdog(unsigned int wdog_id)
{
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_WDOG1 + wdog_id;

	/* Check for error */
	if (wdog_id > MXC_MAX_WDOG_NUM)
		return;

	/* Enable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_id, true);
}

void imx_clock_disable_wdog(unsigned int wdog_id)
{
	unsigned int ccm_trgt_id = CCM_TRT_ID_WDOG_CLK_ROOT;
	unsigned int ccm_ccgr_id = CCM_CCGR_ID_WDOG1 + wdog_id;

	/* Check for error */
	if (wdog_id > MXC_MAX_WDOG_NUM)
		return;

	/* Disable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_id, false);

	/* Clear the target */
	imx_clock_target_clr(ccm_trgt_id, 0xFFFFFFFF);
}

void imx_clock_set_wdog_clk_root_bits(uint32_t wdog_clk_root_en_bits)
{
	/* Enable the common clock root just once */
	imx_clock_target_set(CCM_TRT_ID_WDOG_CLK_ROOT, wdog_clk_root_en_bits);
}

void imx_clock_enable_usb(unsigned int ccm_ccgr_usb_id)
{
	/* Enable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_usb_id, true);
}

void imx_clock_disable_usb(unsigned int ccm_ccgr_usb_id)
{
	/* Disable the clock gate */
	imx_clock_gate_enable(ccm_ccgr_usb_id, false);
}

void imx_clock_set_usb_clk_root_bits(uint32_t usb_clk_root_en_bits)
{
	/* Enable the common clock root just once */
	imx_clock_target_set(CCM_TRT_ID_USB_HSIC_CLK_ROOT, usb_clk_root_en_bits);
}
