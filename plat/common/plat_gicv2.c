/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Portions copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>
#include <plat/common/platform.h>

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
#pragma weak plat_ic_raise_ns_sgi
#pragma weak plat_ic_raise_s_el1_sgi
#pragma weak plat_ic_set_spi_routing

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
	if (id < PENDING_G1_INTID) {
#if GICV2_G0_FOR_EL3
		return INTR_TYPE_EL3;
#else
		return INTR_TYPE_S_EL1;
#endif
	}

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
	return (type == GICV2_INTR_GROUP1) ? INTR_TYPE_NS :
#if GICV2_G0_FOR_EL3
		INTR_TYPE_EL3;
#else
		INTR_TYPE_S_EL1;
#endif
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
	assert((type == INTR_TYPE_S_EL1) || (type == INTR_TYPE_EL3) ||
	       (type == INTR_TYPE_NS));

	assert(sec_state_is_valid(security_state));

	/* Non-secure interrupts are signaled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/*
	 * Secure interrupts are signaled using the IRQ line if the FIQ is
	 * not enabled else they are signaled using the FIQ line.
	 */
	return ((gicv2_is_fiq_enabled() != 0U) ? __builtin_ctz(SCR_FIQ_BIT) :
						 __builtin_ctz(SCR_IRQ_BIT));
}

unsigned int plat_ic_get_running_priority(void)
{
	return gicv2_get_running_priority();
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
	return gicv2_get_interrupt_active(id);
}

void plat_ic_enable_interrupt(unsigned int id)
{
	gicv2_enable_interrupt(id);
}

void plat_ic_disable_interrupt(unsigned int id)
{
	gicv2_disable_interrupt(id);
}

void plat_ic_set_interrupt_priority(unsigned int id, unsigned int priority)
{
	gicv2_set_interrupt_priority(id, priority);
}

int plat_ic_has_interrupt_type(unsigned int type)
{
	int has_interrupt_type = 0;

	switch (type) {
#if GICV2_G0_FOR_EL3
	case INTR_TYPE_EL3:
#else
	case INTR_TYPE_S_EL1:
#endif
	case INTR_TYPE_NS:
		has_interrupt_type = 1;
		break;
	default:
		/* Do nothing in default case */
		break;
	}

	return has_interrupt_type;
}

void plat_ic_set_interrupt_type(unsigned int id, unsigned int type)
{
	unsigned int gicv2_type = 0U;

	/* Map canonical interrupt type to GICv2 type */
	switch (type) {
#if GICV2_G0_FOR_EL3
	case INTR_TYPE_EL3:
#else
	case INTR_TYPE_S_EL1:
#endif
		gicv2_type = GICV2_INTR_GROUP0;
		break;
	case INTR_TYPE_NS:
		gicv2_type = GICV2_INTR_GROUP1;
		break;
	default:
		assert(0); /* Unreachable */
		break;
	}

	gicv2_set_interrupt_type(id, gicv2_type);
}

void plat_ic_raise_el3_sgi(int sgi_num, u_register_t target)
{
#if GICV2_G0_FOR_EL3
	int id;

	/* Target must be a valid MPIDR in the system */
	id = plat_core_pos_by_mpidr(target);
	assert(id >= 0);

	/* Verify that this is a secure SGI */
	assert(plat_ic_get_interrupt_type(sgi_num) == INTR_TYPE_EL3);

	gicv2_raise_sgi(sgi_num, false, id);
#else
	assert(false);
#endif
}

void plat_ic_raise_ns_sgi(int sgi_num, u_register_t target)
{
	int id;

	/* Target must be a valid MPIDR in the system */
	id = plat_core_pos_by_mpidr(target);
	assert(id >= 0);

	/* Verify that this is a non-secure SGI */
	assert(plat_ic_get_interrupt_type(sgi_num) == INTR_TYPE_NS);

	gicv2_raise_sgi(sgi_num, true, id);
}

void plat_ic_raise_s_el1_sgi(int sgi_num, u_register_t target)
{
#if GICV2_G0_FOR_EL3
	assert(false);
#else
	int id;

	/* Target must be a valid MPIDR in the system */
	id = plat_core_pos_by_mpidr(target);
	assert(id >= 0);

	/* Verify that this is a secure EL1 SGI */
	assert(plat_ic_get_interrupt_type(sgi_num) == INTR_TYPE_S_EL1);

	gicv2_raise_sgi(sgi_num, false, id);
#endif
}

void plat_ic_set_spi_routing(unsigned int id, unsigned int routing_mode,
		u_register_t mpidr)
{
	int proc_num = 0;

	switch (routing_mode) {
	case INTR_ROUTING_MODE_PE:
		proc_num = plat_core_pos_by_mpidr(mpidr);
		assert(proc_num >= 0);
		break;
	case INTR_ROUTING_MODE_ANY:
		/* Bit mask selecting all 8 CPUs as candidates */
		proc_num = -1;
		break;
	default:
		assert(0); /* Unreachable */
		break;
	}

	gicv2_set_spi_routing(id, proc_num);
}

void plat_ic_set_interrupt_pending(unsigned int id)
{
	gicv2_set_interrupt_pending(id);
}

void plat_ic_clear_interrupt_pending(unsigned int id)
{
	gicv2_clear_interrupt_pending(id);
}

unsigned int plat_ic_set_priority_mask(unsigned int mask)
{
	return gicv2_set_pmr(mask);
}

unsigned int plat_ic_get_interrupt_id(unsigned int raw)
{
	unsigned int id = (raw & INT_ID_MASK);

	if (id == GIC_SPURIOUS_INTERRUPT)
		id = INTR_ID_UNAVAILABLE;

	return id;
}
