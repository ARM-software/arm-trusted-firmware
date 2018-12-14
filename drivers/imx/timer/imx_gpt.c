/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <imx_gpt.h>

#define GPTCR_SWR		BIT(15)		/* Software reset */
#define GPTCR_24MEN		BIT(10)		/* Enable 24MHz clock input */
#define GPTCR_CLKSOURCE_OSC	(5 << 6)        /* Clock source OSC */
#define GPTCR_CLKSOURCE_MASK	(0x7 << 6)
#define GPTCR_TEN		1		/* Timer enable */

#define GPTPR_PRESCL_24M_SHIFT 12

#define SYS_COUNTER_FREQ_IN_MHZ 3

#define GPTPR_TIMER_CTRL	(imx_base_addr + 0x000)
#define GPTPR_TIMER_PRESCL	(imx_base_addr + 0x004)
#define GPTPR_TIMER_CNTR	(imx_base_addr + 0x024)

static uintptr_t imx_base_addr;

uint32_t imx_get_timer_value(void)
{
	return ~mmio_read_32(GPTPR_TIMER_CNTR);
}

static const timer_ops_t imx_gpt_ops = {
	.get_timer_value	= imx_get_timer_value,
	.clk_mult		= 1,
	.clk_div		= SYS_COUNTER_FREQ_IN_MHZ,
};

void imx_gpt_ops_init(uintptr_t base_addr)
{
	int val;

	assert(base_addr != 0);

	imx_base_addr = base_addr;

	/* setup GP Timer */
	mmio_write_32(GPTPR_TIMER_CTRL, GPTCR_SWR);
	mmio_write_32(GPTPR_TIMER_CTRL, 0);

	/* get 3MHz from 24MHz */
	mmio_write_32(GPTPR_TIMER_PRESCL, (7 << GPTPR_PRESCL_24M_SHIFT));

	val = mmio_read_32(GPTPR_TIMER_CTRL);
	val &= ~GPTCR_CLKSOURCE_MASK;
	val |= GPTCR_24MEN | GPTCR_CLKSOURCE_OSC | GPTCR_TEN;
	mmio_write_32(GPTPR_TIMER_CTRL, val);

	timer_init(&imx_gpt_ops);
}
