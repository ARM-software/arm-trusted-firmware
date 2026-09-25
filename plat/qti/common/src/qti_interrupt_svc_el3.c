/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdint.h>

#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>

#include <platform.h>
#include <qti_interrupt_svc.h>

/*
 * Top-level EL3 interrupt handler.
 */
static uint64_t qti_el3_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	uint32_t irq = QTI_INTR_INVALID_INT_NUM;

	/*
	 * EL3 non-interruptible. Interrupt shouldn't occur when we are at
	 * EL3 / Secure.
	 */
	assert(handle != cm_get_context(SECURE));

	irq = plat_ic_acknowledge_interrupt();

	qti_interrupt_svc_dispatch(irq, handle);

	/* End of Interrupt. */
	if (irq < 1022U) {
		plat_ic_end_of_interrupt(irq);
	}

	return (uint64_t) handle;
}

void qti_interrupt_svc_register_el3_owner(uint64_t flags)
{
	if (register_interrupt_type_handler(INTR_TYPE_EL3,
					    qti_el3_interrupt_handler, flags) != 0) {
		ERROR("Failed to register EL3 interrupt handler\n");
		panic();
	}
}
