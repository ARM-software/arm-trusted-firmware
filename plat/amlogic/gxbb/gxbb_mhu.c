/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/bakery_lock.h>
#include <lib/mmio.h>

static DEFINE_BAKERY_LOCK(mhu_lock);

void mhu_secure_message_start(void)
{
	bakery_lock_get(&mhu_lock);

	while (mmio_read_32(GXBB_HIU_MAILBOX_STAT_3) != 0)
		;
}

void mhu_secure_message_send(uint32_t msg)
{
	mmio_write_32(GXBB_HIU_MAILBOX_SET_3, msg);

	while (mmio_read_32(GXBB_HIU_MAILBOX_STAT_3) != 0)
		;
}

uint32_t mhu_secure_message_wait(void)
{
	uint32_t val;

	do {
		val = mmio_read_32(GXBB_HIU_MAILBOX_STAT_0);
	} while (val == 0);

	return val;
}

void mhu_secure_message_end(void)
{
	mmio_write_32(GXBB_HIU_MAILBOX_CLR_0, 0xFFFFFFFF);

	bakery_lock_release(&mhu_lock);
}

void mhu_secure_init(void)
{
	bakery_lock_init(&mhu_lock);

	mmio_write_32(GXBB_HIU_MAILBOX_CLR_3, 0xFFFFFFFF);
}
