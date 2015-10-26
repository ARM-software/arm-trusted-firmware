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
#include <gic_common.h>
#include <gicv2.h>
#include <interrupt_mgmt.h>

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

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller
 */
uint32_t plat_ic_get_pending_interrupt_id(void)
{
	unsigned int id;

	id = gicv2_get_pending_interrupt_id();
	if (id == GIC_SPURIOUS_INTERRUPT)
		return INTR_ID_UNAVAILABLE;

	return id;
}

/*
 * This function returns the type of the highest priority pending interrupt
 * at the Interrupt controller. In the case of GICv2, the Highest Priority
 * Pending interrupt register (`GICC_HPPIR`) is read to determine the id of
 * the pending interrupt. The type of interrupt depends upon the id value
 * as follows.
 *   1. id < PENDING_G1_INTID (1022) is reported as a S-EL1 interrupt
 *   2. id = PENDING_G1_INTID (1022) is reported as a Non-secure interrupt.
 *   3. id = GIC_SPURIOUS_INTERRUPT (1023) is reported as an invalid interrupt
 *           type.
 */
uint32_t plat_ic_get_pending_interrupt_type(void)
{
	unsigned int id;

	id = gicv2_get_pending_interrupt_type();

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (id < PENDING_G1_INTID)
		return INTR_TYPE_S_EL1;

	if (id == GIC_SPURIOUS_INTERRUPT)
		return INTR_TYPE_INVAL;

	return INTR_TYPE_NS;
}

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller and indicates to the Interrupt controller
 * that the interrupt processing has started.
 */
uint32_t plat_ic_acknowledge_interrupt(void)
{
	return gicv2_acknowledge_interrupt();
}

/*
 * This function returns the type of the interrupt `id`, depending on how
 * the interrupt has been configured in the interrupt controller
 */
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	unsigned int type;

	type = gicv2_get_interrupt_group(id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	return (type) ? INTR_TYPE_NS : INTR_TYPE_S_EL1;
}

/*
 * This functions is used to indicate to the interrupt controller that
 * the processing of the interrupt corresponding to the `id` has
 * finished.
 */
void plat_ic_end_of_interrupt(uint32_t id)
{
	gicv2_end_of_interrupt(id);
}

/*
 * An ARM processor signals interrupt exceptions through the IRQ and FIQ pins.
 * The interrupt controller knows which pin/line it uses to signal a type of
 * interrupt. It lets the interrupt management framework determine
 * for a type of interrupt and security state, which line should be used in the
 * SCR_EL3 to control its routing to EL3. The interrupt line is represented
 * as the bit position of the IRQ or FIQ bit in the SCR_EL3.
 */
uint32_t plat_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
		       type == INTR_TYPE_EL3 ||
		       type == INTR_TYPE_NS);

	/* Non-secure interrupts are signaled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/*
	 * Secure interrupts are signaled using the IRQ line if the FIQ is
	 * not enabled else they are signaled using the FIQ line.
	 */
	return ((gicv2_is_fiq_enabled()) ? __builtin_ctz(SCR_FIQ_BIT) :
						__builtin_ctz(SCR_IRQ_BIT));
}
