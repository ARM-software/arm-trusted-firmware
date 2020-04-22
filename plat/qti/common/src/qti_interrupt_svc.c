/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/gic_common.h>
#include <lib/el3_runtime/context_mgmt.h>

#include <platform.h>
#include <qti_interrupt_svc.h>
#include <qtiseclib_interface.h>

#define QTI_INTR_INVALID_INT_NUM		0xFFFFFFFFU

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

	qtiseclib_invoke_isr(irq, handle);

	/* End of Interrupt. */
	if (irq < 1022U) {
		plat_ic_end_of_interrupt(irq);
	}

	return (uint64_t) handle;
}

int qti_interrupt_svc_init(void)
{
	int ret;
	uint64_t flags = 0U;

	/*
	 * Route EL3 interrupts to EL3 when in Non-secure.
	 * Note: EL3 won't have interrupt enable
	 * & we don't have S-EL1 support.
	 */
	set_interrupt_rm_flag(flags, NON_SECURE);

	/* Register handler for EL3 interrupts */
	ret = register_interrupt_type_handler(INTR_TYPE_EL3,
					      qti_el3_interrupt_handler, flags);
	assert(ret == 0);

	return ret;
}
