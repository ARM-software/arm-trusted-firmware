/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <mmio.h>
#include <stdint.h>
#include <stdbool.h>
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
