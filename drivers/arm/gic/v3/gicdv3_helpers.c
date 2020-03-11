/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "gicv3_private.h"

/*******************************************************************************
 * GIC Distributor interface accessors for bit operations
 ******************************************************************************/

/*
 * Accessor to read the GIC Distributor IGRPMODR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
uint32_t gicd_read_igrpmodr(uintptr_t base, unsigned int id)
{
	return GICD_READ(IGRPMODR, base, id);
}

/*
 * Accessor to write the GIC Distributor IGRPMODR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_igrpmodr(uintptr_t base, unsigned int id, uint32_t val)
{
	GICD_WRITE(IGRPMODR, base, id, val);
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id)
{
	return GICD_GET_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
void gicd_set_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_SET_BIT(IGRPMODR, base, id);
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_CLR_BIT(IGRPMODR, base, id);
}
