/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <stdint.h>

#include <qti_interrupt_svc.h>

/*
 * SPMD group-0 interrupt handler. SPMD calls this when a Group0 (EL3)
 * interrupt fires while the normal world is running. Forward it through
 * the QTI ISR dispatch table so registered qti_interrupt_svc callbacks
 * still fire correctly. Return 0 to tell SPMD the interrupt was handled;
 * SPMD owns the EOI and will call plat_ic_end_of_interrupt() itself.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	qti_interrupt_svc_dispatch(intid, NULL);
	return 0;
}

/*
 * When SPD=spmd, the SPMD framework registers INTR_TYPE_EL3 itself and
 * dispatches group-0 interrupts via plat_spmd_handle_group0_interrupt()
 * above, so QTI has no EL3 owner of its own to register here.
 */
void qti_interrupt_svc_register_el3_owner(uint64_t flags)
{
	(void)flags;
}
