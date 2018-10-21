/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <drivers/delay_timer.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <sunxi_mmap.h>

#define REMOTE_IRQ_EN_REG	0x0040
#define REMOTE_IRQ_STAT_REG	0x0050
#define LOCAL_IRQ_EN_REG	0x0060
#define LOCAL_IRQ_STAT_REG	0x0070

#define RX_IRQ(n)		BIT(0 + 2 * (n))
#define TX_IRQ(n)		BIT(1 + 2 * (n))

#define FIFO_STAT_REG(n)	(0x0100 + 0x4 * (n))
#define FIFO_STAT_MASK		GENMASK(0, 0)

#define MSG_STAT_REG(n)		(0x0140 + 0x4 * (n))
#define MSG_STAT_MASK		GENMASK(2, 0)

#define MSG_DATA_REG(n)		(0x0180 + 0x4 * (n))

#define RX_CHAN			1
#define TX_CHAN			0

#define MHU_MAX_SLOT_ID		31

#define MHU_TIMEOUT_DELAY	10
#define MHU_TIMEOUT_ITERS	10000

static DEFINE_BAKERY_LOCK(mhu_secure_message_lock);

static bool sunxi_msgbox_last_tx_done(unsigned int chan)
{
	uint32_t stat = mmio_read_32(SUNXI_MSGBOX_BASE + REMOTE_IRQ_STAT_REG);

	return (stat & RX_IRQ(chan)) == 0U;
}

static bool sunxi_msgbox_peek_data(unsigned int chan)
{
	uint32_t stat = mmio_read_32(SUNXI_MSGBOX_BASE + MSG_STAT_REG(chan));

	return (stat & MSG_STAT_MASK) != 0U;
}

void mhu_secure_message_start(unsigned int slot_id __unused)
{
	uint32_t timeout = MHU_TIMEOUT_ITERS;

	bakery_lock_get(&mhu_secure_message_lock);

	/* Wait for all previous messages to be acknowledged. */
	while (!sunxi_msgbox_last_tx_done(TX_CHAN) && --timeout)
		udelay(MHU_TIMEOUT_DELAY);
}

void mhu_secure_message_send(unsigned int slot_id)
{
	mmio_write_32(SUNXI_MSGBOX_BASE + MSG_DATA_REG(TX_CHAN), BIT(slot_id));
}

uint32_t mhu_secure_message_wait(void)
{
	uint32_t timeout = MHU_TIMEOUT_ITERS;
	uint32_t msg = 0;

	/* Wait for a message from the SCP. */
	while (!sunxi_msgbox_peek_data(RX_CHAN) && --timeout)
		udelay(MHU_TIMEOUT_DELAY);

	/* Return the most recent message in the FIFO. */
	while (sunxi_msgbox_peek_data(RX_CHAN))
		msg = mmio_read_32(SUNXI_MSGBOX_BASE + MSG_DATA_REG(RX_CHAN));

	return msg;
}

void mhu_secure_message_end(unsigned int slot_id)
{
	/* Acknowledge a response by clearing the IRQ status. */
	mmio_write_32(SUNXI_MSGBOX_BASE + LOCAL_IRQ_STAT_REG, RX_IRQ(RX_CHAN));

	bakery_lock_release(&mhu_secure_message_lock);
}
