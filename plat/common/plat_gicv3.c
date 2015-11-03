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
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cassert.h>
#include <gic_common.h>
#include <gicv3.h>
#include <interrupt_mgmt.h>
#include <platform.h>

#if IMAGE_BL31

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

CASSERT((INTR_TYPE_S_EL1 == INTR_GROUP1S) &&
	(INTR_TYPE_NS == INTR_GROUP1NS) &&
	(INTR_TYPE_EL3 == INTR_GROUP0), assert_interrupt_type_mismatch);

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller
 */
uint32_t plat_ic_get_pending_interrupt_id(void)
{
	unsigned int irqnr;

	assert(IS_IN_EL3());
	irqnr = gicv3_get_pending_interrupt_id();
	return (gicv3_is_intr_id_special_identifier(irqnr)) ?
				INTR_ID_UNAVAILABLE : irqnr;
}

/*
 * This function returns the type of the highest priority pending interrupt
 * at the Interrupt controller. In the case of GICv3, the Highest Priority
 * Pending interrupt system register (`ICC_HPPIR0_EL1`) is read to determine
 * the id of the pending interrupt. The type of interrupt depends upon the
 * id value as follows.
 *   1. id = PENDING_G1S_INTID (1020) is reported as a S-EL1 interrupt
 *   2. id = PENDING_G1NS_INTID (1021) is reported as a Non-secure interrupt.
 *   3. id = GIC_SPURIOUS_INTERRUPT (1023) is reported as an invalid interrupt
 *           type.
 *   4. All other interrupt id's are reported as EL3 interrupt.
 */
uint32_t plat_ic_get_pending_interrupt_type(void)
{
	unsigned int irqnr;

	assert(IS_IN_EL3());
	irqnr = gicv3_get_pending_interrupt_type();

	switch (irqnr) {
	case PENDING_G1S_INTID:
		return INTR_TYPE_S_EL1;
	case PENDING_G1NS_INTID:
		return INTR_TYPE_NS;
	case GIC_SPURIOUS_INTERRUPT:
		return INTR_TYPE_INVAL;
	default:
		return INTR_TYPE_EL3;
	}
}

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller and indicates to the Interrupt controller
 * that the interrupt processing has started.
 */
uint32_t plat_ic_acknowledge_interrupt(void)
{
	assert(IS_IN_EL3());
	return gicv3_acknowledge_interrupt();
}

/*
 * This function returns the type of the interrupt `id`, depending on how
 * the interrupt has been configured in the interrupt controller
 */
uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	assert(IS_IN_EL3());
	return gicv3_get_interrupt_type(id, plat_my_core_pos());
}

/*
 * This functions is used to indicate to the interrupt controller that
 * the processing of the interrupt corresponding to the `id` has
 * finished.
 */
void plat_ic_end_of_interrupt(uint32_t id)
{
	assert(IS_IN_EL3());
	gicv3_end_of_interrupt(id);
}

/*
 * An ARM processor signals interrupt exceptions through the IRQ and FIQ pins.
 * The interrupt controller knows which pin/line it uses to signal a type of
 * interrupt. It lets the interrupt management framework determine for a type of
 * interrupt and security state, which line should be used in the SCR_EL3 to
 * control its routing to EL3. The interrupt line is represented as the bit
 * position of the IRQ or FIQ bit in the SCR_EL3.
 */
uint32_t plat_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(sec_state_is_valid(security_state));
	assert(IS_IN_EL3());

	switch (type) {
	case INTR_TYPE_S_EL1:
		/*
		 * The S-EL1 interrupts are signaled as IRQ in S-EL0/1 contexts
		 * and as FIQ in the NS-EL0/1/2 contexts
		 */
		if (security_state == SECURE)
			return __builtin_ctz(SCR_IRQ_BIT);
		else
			return __builtin_ctz(SCR_FIQ_BIT);
	case INTR_TYPE_NS:
		/*
		 * The Non secure interrupts will be signaled as FIQ in S-EL0/1
		 * contexts and as IRQ in the NS-EL0/1/2 contexts.
		 */
		if (security_state == SECURE)
			return __builtin_ctz(SCR_FIQ_BIT);
		else
			return __builtin_ctz(SCR_IRQ_BIT);
	default:
		assert(0);
		/* Fall through in the release build */
	case INTR_TYPE_EL3:
		/*
		 * The EL3 interrupts are signaled as FIQ in both S-EL0/1 and
		 * NS-EL0/1/2 contexts
		 */
		return __builtin_ctz(SCR_FIQ_BIT);
	}
}
#endif
#if IMAGE_BL32

#pragma weak plat_ic_get_pending_interrupt_id
#pragma weak plat_ic_acknowledge_interrupt
#pragma weak plat_ic_end_of_interrupt

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller
 */
uint32_t plat_ic_get_pending_interrupt_id(void)
{
	unsigned int irqnr;

	assert(IS_IN_EL1());
	irqnr = gicv3_get_pending_interrupt_id_sel1();
	return (irqnr == GIC_SPURIOUS_INTERRUPT) ?
				INTR_ID_UNAVAILABLE : irqnr;
}

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller and indicates to the Interrupt controller
 * that the interrupt processing has started.
 */
uint32_t plat_ic_acknowledge_interrupt(void)
{
	assert(IS_IN_EL1());
	return gicv3_acknowledge_interrupt_sel1();
}

/*
 * This functions is used to indicate to the interrupt controller that
 * the processing of the interrupt corresponding to the `id` has
 * finished.
 */
void plat_ic_end_of_interrupt(uint32_t id)
{
	assert(IS_IN_EL1());
	gicv3_end_of_interrupt_sel1(id);
}
#endif
