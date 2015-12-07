/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <gicv2.h>
#include <interrupt_mgmt.h>

uint32_t plat_ic_get_pending_interrupt_id(void)
{
	return gicv2_get_pending_interrupt_id();
}

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	return gicv2_get_pending_interrupt_type();
}

uint32_t plat_ic_acknowledge_interrupt(void)
{
	return gicv2_acknowledge_interrupt();
}

uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicv2_get_interrupt_group(id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (!group)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;

}

void plat_ic_end_of_interrupt(uint32_t id)
{
	gicv2_end_of_interrupt(id);
}

uint32_t plat_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(sec_state_is_valid(security_state));

	/* Non-secure interrupts are signalled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/*
	 * Secure interrupts are signalled using the IRQ line if the FIQ_EN
	 * bit is not set else they are signalled using the FIQ line.
	 */
	if (gicv2_is_fiq_enabled())
		return __builtin_ctz(SCR_FIQ_BIT);
	else
		return __builtin_ctz(SCR_IRQ_BIT);
}

