/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/cassert.h>
#include <plat/common/platform.h>

#ifdef IMAGE_BL31

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

#pragma weak plat_ic_get_running_priority
#pragma weak plat_ic_is_spi
#pragma weak plat_ic_is_ppi
#pragma weak plat_ic_is_sgi
#pragma weak plat_ic_get_interrupt_active
#pragma weak plat_ic_enable_interrupt
#pragma weak plat_ic_disable_interrupt
#pragma weak plat_ic_set_interrupt_priority
#pragma weak plat_ic_set_interrupt_type
#pragma weak plat_ic_raise_el3_sgi
#pragma weak plat_ic_set_spi_routing
#pragma weak plat_ic_set_interrupt_pending
#pragma weak plat_ic_clear_interrupt_pending

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
	return gicv3_is_intr_id_special_identifier(irqnr) ?
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
	uint32_t type;

	assert(IS_IN_EL3());
	irqnr = gicv3_get_pending_interrupt_type();

	switch (irqnr) {
	case PENDING_G1S_INTID:
		type = INTR_TYPE_S_EL1;
		break;
	case PENDING_G1NS_INTID:
		type = INTR_TYPE_NS;
		break;
	case GIC_SPURIOUS_INTERRUPT:
		type = INTR_TYPE_INVAL;
		break;
	default:
		type = INTR_TYPE_EL3;
		break;
	}

	return type;
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
	assert((type == INTR_TYPE_S_EL1) ||
	       (type == INTR_TYPE_EL3) ||
	       (type == INTR_TYPE_NS));

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
		assert(0); /* Unreachable */
	case INTR_TYPE_NS:
		/*
		 * The Non secure interrupts will be signaled as FIQ in S-EL0/1
		 * contexts and as IRQ in the NS-EL0/1/2 contexts.
		 */
		if (security_state == SECURE)
			return __builtin_ctz(SCR_FIQ_BIT);
		else
			return __builtin_ctz(SCR_IRQ_BIT);
		assert(0); /* Unreachable */
	case INTR_TYPE_EL3:
		/*
		 * The EL3 interrupts are signaled as FIQ in both S-EL0/1 and
		 * NS-EL0/1/2 contexts
		 */
		return __builtin_ctz(SCR_FIQ_BIT);
	default:
		panic();
	}
}

unsigned int plat_ic_get_running_priority(void)
{
	return gicv3_get_running_priority();
}

int plat_ic_is_spi(unsigned int id)
{
	return (id >= MIN_SPI_ID) && (id <= MAX_SPI_ID);
}

int plat_ic_is_ppi(unsigned int id)
{
	return (id >= MIN_PPI_ID) && (id < MIN_SPI_ID);
}

int plat_ic_is_sgi(unsigned int id)
{
	return (id >= MIN_SGI_ID) && (id < MIN_PPI_ID);
}

unsigned int plat_ic_get_interrupt_active(unsigned int id)
{
	return gicv3_get_interrupt_active(id, plat_my_core_pos());
}

void plat_ic_enable_interrupt(unsigned int id)
{
	gicv3_enable_interrupt(id, plat_my_core_pos());
}

void plat_ic_disable_interrupt(unsigned int id)
{
	gicv3_disable_interrupt(id, plat_my_core_pos());
}

void plat_ic_set_interrupt_priority(unsigned int id, unsigned int priority)
{
	gicv3_set_interrupt_priority(id, plat_my_core_pos(), priority);
}

int plat_ic_has_interrupt_type(unsigned int type)
{
	assert((type == INTR_TYPE_EL3) || (type == INTR_TYPE_S_EL1) ||
			(type == INTR_TYPE_NS));
	return 1;
}

void plat_ic_set_interrupt_type(unsigned int id, unsigned int type)
{
	gicv3_set_interrupt_type(id, plat_my_core_pos(), type);
}

void plat_ic_raise_el3_sgi(int sgi_num, u_register_t target)
{
	/* Target must be a valid MPIDR in the system */
	assert(plat_core_pos_by_mpidr(target) >= 0);

	/* Verify that this is a secure EL3 SGI */
	assert(plat_ic_get_interrupt_type((unsigned int)sgi_num) ==
					  INTR_TYPE_EL3);

	gicv3_raise_secure_g0_sgi((unsigned int)sgi_num, target);
}

void plat_ic_set_spi_routing(unsigned int id, unsigned int routing_mode,
		u_register_t mpidr)
{
	unsigned int irm = 0;

	switch (routing_mode) {
	case INTR_ROUTING_MODE_PE:
		assert(plat_core_pos_by_mpidr(mpidr) >= 0);
		irm = GICV3_IRM_PE;
		break;
	case INTR_ROUTING_MODE_ANY:
		irm = GICV3_IRM_ANY;
		break;
	default:
		assert(0); /* Unreachable */
		break;
	}

	gicv3_set_spi_routing(id, irm, mpidr);
}

void plat_ic_set_interrupt_pending(unsigned int id)
{
	/* Disallow setting SGIs pending */
	assert(id >= MIN_PPI_ID);
	gicv3_set_interrupt_pending(id, plat_my_core_pos());
}

void plat_ic_clear_interrupt_pending(unsigned int id)
{
	/* Disallow setting SGIs pending */
	assert(id >= MIN_PPI_ID);
	gicv3_clear_interrupt_pending(id, plat_my_core_pos());
}

unsigned int plat_ic_set_priority_mask(unsigned int mask)
{
	return gicv3_set_pmr(mask);
}

unsigned int plat_ic_get_interrupt_id(unsigned int raw)
{
	unsigned int id = raw & INT_ID_MASK;

	return gicv3_is_intr_id_special_identifier(id) ?
			INTR_ID_UNAVAILABLE : id;
}
#endif
#ifdef IMAGE_BL32

#pragma weak plat_ic_get_pending_interrupt_id
#pragma weak plat_ic_acknowledge_interrupt
#pragma weak plat_ic_end_of_interrupt

/* In AArch32, the secure group1 interrupts are targeted to Secure PL1 */
#ifndef __aarch64__
#define IS_IN_EL1()	IS_IN_SECURE()
#endif

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
