/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <ls_interrupt_mgmt.h>
#include <plat/common/platform.h>

static interrupt_type_handler_t type_el3_interrupt_table[MAX_INTR_EL3];

int request_intr_type_el3(uint32_t id, interrupt_type_handler_t handler)
{
	/* Validate 'handler' and 'id' parameters */
	if (!handler || id >= MAX_INTR_EL3) {
		return -EINVAL;
	}

	/* Check if a handler has already been registered */
	if (type_el3_interrupt_table[id] != NULL) {
		return -EALREADY;
	}

	type_el3_interrupt_table[id] = handler;

	return 0;
}

static uint64_t ls_el3_interrupt_handler(uint32_t id, uint32_t flags,
					  void *handle, void *cookie)
{
	uint32_t intr_id;
	interrupt_type_handler_t handler;

	intr_id = plat_ic_get_pending_interrupt_id();

	INFO("Interrupt recvd is %d\n", intr_id);

	handler = type_el3_interrupt_table[intr_id];
	if (handler != NULL) {
		handler(intr_id, flags, handle, cookie);
	}

	/*
	 * Mark this interrupt as complete to avoid a interrupt storm.
	 */
	plat_ic_end_of_interrupt(intr_id);

	return 0U;
}

void ls_el3_interrupt_config(void)
{
	uint64_t flags = 0U;
	uint64_t rc;

	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					     ls_el3_interrupt_handler, flags);
	if (rc != 0U) {
		panic();
	}
}
