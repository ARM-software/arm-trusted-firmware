/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
