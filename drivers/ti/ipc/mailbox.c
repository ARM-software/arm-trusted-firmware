/*
 * Texas Instruments Mailbox Driver
 *
 * Copyright (C) 2024-2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/utils_def.h>
#include <ti_sci_transport.h>

#include <platform_def.h>

/*
 * TI_MAILBOX_RX/TX_BASE and the MAILBOX_MAX_MESSAGE_SIZE values are expected
 * to come from platform specific header file ie. platform_def.h
 */

#define TI_MAILBOX_SYSC		UL(0x10)
#define TI_MAILBOX_MSG		UL(0x40)
#define TI_MAILBOX_FIFO_STATUS	UL(0x80)
#define TI_MAILBOX_MSG_STATUS		UL(0xc0)

/*
 * Function to poll for mailbox rx messages
 * IRQ model is currently not in scope of this driver
 */
static int8_t ti_mailbox_poll_rx_status(void)
{
	uint32_t num_messages_pending = 0U;
	uint32_t retry_count = 100U;

	/*
	 * Keep polling till we get a message for 100 times
	 * with intervals of 10 milliseconds.
	 */
	while (num_messages_pending == 0U) {
		num_messages_pending = mmio_read_32(TI_MAILBOX_RX_BASE + TI_MAILBOX_MSG_STATUS);
		if (retry_count-- == 0U) {
			return -ETIMEDOUT;
		}
		mdelay(10);
	}
	return 0;
}

int ti_sci_transport_clear_rx_thread(enum ti_sci_transport_chan_id id)
{
	/* MSG_STATUS tells us how many pending messages */
	uint32_t try_count = mmio_read_32(TI_MAILBOX_RX_BASE + TI_MAILBOX_MSG_STATUS);

	/* Run the loop till the status register is cleared */
	while (mmio_read_32(TI_MAILBOX_RX_BASE + TI_MAILBOX_MSG_STATUS) != 0U) {
		WARN("Clearing message from mailbox FIFO\n");
		/* The act of reading the mailbox msg itself clears it */
		mmio_read_32(TI_MAILBOX_RX_BASE + TI_MAILBOX_MSG);
		/*
		 * The try_count is kept independent of the value of the status register
		 * because if at any point a new mailbox message arrives while this loop
		 * is in progress, we would want to know that message arrived and not clear
		 * it. We would rather print the error than clear the message thus indicating
		 * that the system is probably in a bad/async state.
		 */
		if (!(try_count--)) {
			ERROR("Could not clear all messages from mailbox FIFO\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

int ti_sci_transport_send(enum ti_sci_transport_chan_id id, const struct ti_sci_msg *msg)
{
	uint32_t num_bytes;
	void *dst_ptr = (void *)MAILBOX_TX_START_REGION;

	assert(msg != NULL);

	num_bytes = msg->len;

	/*
	 * Only a simple check because even if there's 1 pending message
	 * we will be in a bad state if we try to send another message
	 * due to the absence of any interrupt or buffer mgmt model.
	 */
	if (mmio_read_32(TI_MAILBOX_TX_BASE + TI_MAILBOX_FIFO_STATUS)) {
		ERROR("Mailbox FIFO has pending messages!\n");
		return -EINVAL;
	}

	if (num_bytes > MAILBOX_MAX_MESSAGE_SIZE) {
		ERROR("message length %lu > max msg size\n", msg->len);
		return -EINVAL;
	}

	/*
	 * Move the buffer contents into the SRAM to be accessed by TIFS
	 */
	memmove(dst_ptr, msg->buf, num_bytes);

	mmio_write_32(TI_MAILBOX_TX_BASE + TI_MAILBOX_MSG, (uint64_t)(void *)dst_ptr);

	return 0;
}

int ti_sci_transport_recv(enum ti_sci_transport_chan_id id, struct ti_sci_msg *msg)
{
	uint32_t num_bytes;
	uint64_t rcv_addr;

	assert(msg != NULL);

	num_bytes = msg->len;

	if (ti_mailbox_poll_rx_status() == -ETIMEDOUT) {
		ERROR("Timeout waiting for receive\n");
		return -ETIMEDOUT;
	}

	rcv_addr = mmio_read_32(TI_MAILBOX_RX_BASE + TI_MAILBOX_MSG);

	/*
	 * According to the TI SCI mailbox IPC design, the received message will
	 * always lie in the fixed memory buffer region dedicated for IPC.
	 * There values are defined in board specific board_def.h
	 */
	if (rcv_addr < MAILBOX_RX_START_REGION ||
	    rcv_addr > (MAILBOX_RX_START_REGION +
	    MAILBOX_RX_SLOT_SZ * (MAILBOX_RX_NUM_SLOTS - 1))) {
		ERROR("message address %lu is not valid\n", rcv_addr);
		return -EFAULT;
	}

	/* Ensure that the address is aligned as well */
	if ((rcv_addr - MAILBOX_RX_START_REGION) % MAILBOX_RX_SLOT_SZ != 0) {
		ERROR("message address %lu is not aligned\n", rcv_addr);
		return -EINVAL;
	}

	if (num_bytes > MAILBOX_MAX_MESSAGE_SIZE) {
		ERROR("message length %lu > max msg size\n", msg->len);
		return -EINVAL;
	}

	memmove(msg->buf, (uint8_t *)(rcv_addr), num_bytes);

	return 0;
}
