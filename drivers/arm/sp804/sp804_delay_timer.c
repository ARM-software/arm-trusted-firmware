/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <delay_timer.h>
#include <mmio.h>

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
