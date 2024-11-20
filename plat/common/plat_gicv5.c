/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/gicv5.h>

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	return gicv5_get_pending_interrupt_type();
}

bool plat_ic_has_interrupt_type(unsigned int type)
{
	return gicv5_has_interrupt_type(type);
}
