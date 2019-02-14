/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/sp804_delay_timer.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

uintptr_t sp804_base_addr;

#define SP804_TIMER1_LOAD	(sp804_base_addr + 0x000)
#define SP804_TIMER1_VALUE	(sp804_base_addr + 0x004)
#define SP804_TIMER1_CONTROL	(sp804_base_addr + 0x008)
#define SP804_TIMER1_BGLOAD	(sp804_base_addr + 0x018)

#define TIMER_CTRL_ONESHOT	(1 << 0)
#define TIMER_CTRL_32BIT	(1 << 1)
#define TIMER_CTRL_DIV1		(0 << 2)
#define TIMER_CTRL_DIV16	(1 << 2)
#define TIMER_CTRL_DIV256	(2 << 2)
#define TIMER_CTRL_IE		(1 << 5)
#define TIMER_CTRL_PERIODIC	(1 << 6)
#define TIMER_CTRL_ENABLE	(1 << 7)

/********************************************************************
 * The SP804 timer delay function
 ********************************************************************/
uint32_t sp804_get_timer_value(void)
{
	return mmio_read_32(SP804_TIMER1_VALUE);
}

/********************************************************************
 * Initialize the 1st timer in the SP804 dual timer with a base
 * address and a timer ops
 ********************************************************************/
void sp804_timer_ops_init(uintptr_t base_addr, const timer_ops_t *ops)
{
	assert(base_addr != 0);
	assert(ops != 0 && ops->get_timer_value == sp804_get_timer_value);

	sp804_base_addr = base_addr;
	timer_init(ops);

	/* disable timer1 */
	mmio_write_32(SP804_TIMER1_CONTROL, 0);
	mmio_write_32(SP804_TIMER1_LOAD, UINT32_MAX);
	mmio_write_32(SP804_TIMER1_VALUE, UINT32_MAX);

	/* enable as a free running 32-bit counter */
	mmio_write_32(SP804_TIMER1_CONTROL,
			TIMER_CTRL_32BIT | TIMER_CTRL_ENABLE);
}
