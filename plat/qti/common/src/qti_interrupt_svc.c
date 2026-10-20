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
#include <common/debug.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>

#include <platform.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>

#define ISR_TABLE_LEN				20

static struct qti_isr_table {
	struct qti_isr {
		qti_int_svc_isr_t func;
		uint32_t id;
		void *ctx;
	} entry[ISR_TABLE_LEN];

	spinlock_t lock;
	uint8_t cnt;
} isr_table = {
	.entry[0 ... ISR_TABLE_LEN - 1] = { .id = QTI_INTR_INVALID_INT_NUM },
};

int qti_interrupt_svc_register(uint32_t id, qti_int_svc_isr_t func, void *ctx)
{
	struct qti_isr *p = isr_table.entry;

	spin_lock(&isr_table.lock);
	if (isr_table.cnt >= ISR_TABLE_LEN)
		goto error;

	for (size_t i = 0; i < ISR_TABLE_LEN; i++, p++) {
		if (p->id != QTI_INTR_INVALID_INT_NUM) {
			continue;
		}

		p->func = func;
		p->ctx = ctx;
		p->id = id;
		isr_table.cnt++;
		spin_unlock(&isr_table.lock);

		return 0;
	}
error:
	spin_unlock(&isr_table.lock);

	return -ENOTCAPABLE;
}

int qti_interrupt_svc_unregister(uint32_t id)
{
	struct qti_isr *p = isr_table.entry;

	spin_lock(&isr_table.lock);
	if (!isr_table.cnt)
		goto error;

	for (size_t i = 0; i < ISR_TABLE_LEN; i++, p++) {
		if (p->id != id) {
			continue;
		}

		memset(p, 0, sizeof(*p));
		p->id = QTI_INTR_INVALID_INT_NUM;
		isr_table.cnt--;
		spin_unlock(&isr_table.lock);

		return 0;
	}
error:
	spin_unlock(&isr_table.lock);

	return -ENOENT;
}

void qti_interrupt_svc_dispatch(uint32_t id, void *handle)
{
	struct qti_isr *p = isr_table.entry;
	qti_int_svc_isr_t invoke_isr = NULL;

	spin_lock(&isr_table.lock);
	if (!isr_table.cnt)
		goto plat_dispatch;

	for (size_t i = 0; i < ISR_TABLE_LEN; i++, p++) {
		if (p->id != id) {
			continue;
		}

		invoke_isr = p->func;
		spin_unlock(&isr_table.lock);
		p->ctx = invoke_isr(id, p->ctx);

		return;
	}

plat_dispatch:
	spin_unlock(&isr_table.lock);
	plat_qti_invoke_unhandled_isr(id, handle);
}

int qti_interrupt_svc_init(bool have_sel1)
{
	uint64_t flags = 0U;

	/*
	 * Route EL3 interrupts to EL3 when in Non-secure.
	 * Note: EL3 won't have interrupts enabled.
	 * When we have a Secure EL1 interrupt handler, allow it
	 * to handle Secure interrupts.
	 */
	set_interrupt_rm_flag(flags, NON_SECURE);
	if (!have_sel1)
		set_interrupt_rm_flag(flags, SECURE);

	qti_interrupt_svc_register_el3_owner(flags);

	return 0;
}
