/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
#include <arm_gic.h>

/*
 * The following platform GIC functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak plat_ic_get_pending_interrupt_id
#pragma weak plat_ic_get_pending_interrupt_type
#pragma weak plat_ic_acknowledge_interrupt
#pragma weak plat_ic_get_interrupt_type
#pragma weak plat_ic_end_of_interrupt
#pragma weak plat_interrupt_type_to_line

uint32_t plat_ic_get_pending_interrupt_id(void)
{
	return arm_gic_get_pending_interrupt_id();
}

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	return arm_gic_get_pending_interrupt_type();
}

uint32_t plat_ic_acknowledge_interrupt(void)
{
	return arm_gic_acknowledge_interrupt();
}

uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	return arm_gic_get_interrupt_type(id);
}

void plat_ic_end_of_interrupt(uint32_t id)
{
	arm_gic_end_of_interrupt(id);
}

uint32_t plat_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	return arm_gic_interrupt_type_to_line(type, security_state);
}
