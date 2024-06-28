/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <drivers/arm/mhu.h>

#include "mhu_v3_x.h"

#define MHU_NOTIFY_VALUE	U(1234)

#ifndef ALIGN_UP
#define ALIGN_UP(num, align)	(((num) + ((align) - 1)) & ~((align) - 1))
#endif

/*
 * MHUv3 Wrapper utility macros
 */
#define IS_ALIGNED(val, align)	(val == ALIGN_UP(val, align))

/*
 * MHU devices for host:
 * HSE: Host to Secure Enclave (sender device)
 * SEH: Secure Enclave to Host (receiver device)
 */
struct mhu_v3_x_dev_t mhu_hse_dev = {0, MHU_V3_X_PBX_FRAME};
struct mhu_v3_x_dev_t mhu_seh_dev = {0, MHU_V3_X_MBX_FRAME};

/* MHUv3 driver error to MHUv3 wrapper error mapping */
static enum mhu_error_t error_mapping_to_mhu_error_t(enum mhu_v3_x_error_t err)
{
	switch (err) {
	case MHU_V_3_X_ERR_NONE:
		return MHU_ERR_NONE;

	case MHU_V_3_X_ERR_NOT_INIT:
		return MHU_ERR_NOT_INIT;

	case MHU_V_3_X_ERR_UNSUPPORTED_VERSION:
		return MHU_ERR_UNSUPPORTED_VERSION;

	case MHU_V_3_X_ERR_UNSUPPORTED:
		return MHU_ERR_UNSUPPORTED;

	case MHU_V_3_X_ERR_INVALID_PARAM:
		return MHU_ERR_INVALID_ARG;

	default:
		return MHU_ERR_GENERAL;
	}
}

static enum mhu_error_t signal_and_wait_for_clear(
	void *mhu_sender_dev, uint32_t value)
{
	enum mhu_v3_x_error_t err;
	struct mhu_v3_x_dev_t *dev;
	uint8_t num_channels;
	uint32_t read_val;

	dev = (struct mhu_v3_x_dev_t *)mhu_sender_dev;

	if ((dev == NULL) || (dev->base == 0)) {
		return MHU_ERR_INVALID_ARG;
	}

	err = mhu_v3_x_get_num_channel_implemented(dev,
			MHU_V3_X_CHANNEL_TYPE_DBCH, &num_channels);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* Wait for any pending acknowledgment from transmitter side */
	do {
		err = mhu_v3_x_doorbell_read(dev, num_channels - 1, &read_val);
		if (err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	} while ((read_val & value) == value);

	/* Use the last channel to notify that a transfer is ready */
	err = mhu_v3_x_doorbell_write(dev, num_channels - 1, value);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* Wait until receiver side acknowledges the transfer */
	do {
		err = mhu_v3_x_doorbell_read(dev, num_channels - 1, &read_val);
		if (err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	} while ((read_val & value) == value);

	return error_mapping_to_mhu_error_t(MHU_V_3_X_ERR_NONE);
}

static enum mhu_error_t wait_for_signal(
	void *mhu_receiver_dev, uint32_t value)
{
	enum mhu_v3_x_error_t err;
	struct mhu_v3_x_dev_t *dev;
	uint32_t read_val;
	uint8_t num_channels;

	dev = (struct mhu_v3_x_dev_t *)mhu_receiver_dev;

	if ((dev == NULL) || (dev->base == 0)) {
		return MHU_ERR_INVALID_ARG;
	}

	err = mhu_v3_x_get_num_channel_implemented(dev,
			MHU_V3_X_CHANNEL_TYPE_DBCH, &num_channels);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	do {
		err = mhu_v3_x_doorbell_read(dev, num_channels - 1, &read_val);
		if (err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	} while (read_val != value);

	return error_mapping_to_mhu_error_t(err);
}

static enum mhu_error_t clear_and_wait_for_signal(
	void *mhu_receiver_dev, uint32_t value)
{
	enum mhu_v3_x_error_t err;
	struct mhu_v3_x_dev_t *dev;
	uint8_t num_channels;

	dev = (struct mhu_v3_x_dev_t *)mhu_receiver_dev;

	if ((dev == NULL) || (dev->base == 0)) {
		return MHU_ERR_INVALID_ARG;
	}

	err = mhu_v3_x_get_num_channel_implemented(dev,
			MHU_V3_X_CHANNEL_TYPE_DBCH, &num_channels);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* Clear all channels */
	for (int i = 0; i < num_channels; i++) {
		err = mhu_v3_x_doorbell_clear(dev, i, UINT32_MAX);
		if (err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	}

	return wait_for_signal(mhu_receiver_dev, value);
}

static enum mhu_error_t validate_buffer_params(uintptr_t buf_addr)
{
	if ((buf_addr == 0) || (!IS_ALIGNED(buf_addr, sizeof(uint32_t)))) {
		return MHU_ERR_INVALID_ARG;
	}

	return MHU_ERR_NONE;
}

enum mhu_error_t mhu_init_sender(uintptr_t mhu_sender_base)
{
	enum mhu_v3_x_error_t err;
	struct mhu_v3_x_dev_t *dev;
	uint8_t num_ch;
	uint32_t ch;

	assert(mhu_sender_base != (uintptr_t)NULL);

	mhu_hse_dev.base = mhu_sender_base;
	dev = (struct mhu_v3_x_dev_t *)&mhu_hse_dev;

	/* Initialize MHUv3 */
	err = mhu_v3_x_driver_init(dev);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* Read the number of doorbell channels implemented in the MHU */
	err = mhu_v3_x_get_num_channel_implemented(
		dev, MHU_V3_X_CHANNEL_TYPE_DBCH, &num_ch);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	} else if (num_ch < 2) {
		/* This wrapper requires at least two channels implemented */
		return MHU_ERR_UNSUPPORTED;
	}

	/*
	 * The sender polls the postbox doorbell channel window status register
	 * to get notified about successful transfer. So, disable the doorbell
	 * channel's contribution to postbox combined interrupt.
	 *
	 * Also, clear and disable the postbox doorbell channel transfer
	 * acknowledge interrupt.
	 */
	for (ch = 0; ch < num_ch; ch++) {
		err = mhu_v3_x_channel_interrupt_disable(
			dev, ch, MHU_V3_X_CHANNEL_TYPE_DBCH);
		if (err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	}

	return MHU_ERR_NONE;
}

enum mhu_error_t mhu_init_receiver(uintptr_t mhu_receiver_base)
{
	enum mhu_v3_x_error_t err;
	struct mhu_v3_x_dev_t *dev;
	uint32_t ch;
	uint8_t num_ch;

	assert(mhu_receiver_base != (uintptr_t)NULL);

	mhu_seh_dev.base = mhu_receiver_base;
	dev = (struct mhu_v3_x_dev_t *)&mhu_seh_dev;

	/* Initialize MHUv3 */
	err = mhu_v3_x_driver_init(dev);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* Read the number of doorbell channels implemented in the MHU */
	err = mhu_v3_x_get_num_channel_implemented(
		dev, MHU_V3_X_CHANNEL_TYPE_DBCH, &num_ch);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	} else if (num_ch < 2) {
		/* This wrapper requires at least two channels implemented */
		return MHU_ERR_UNSUPPORTED;
	}

	/* Mask all channels except the notifying channel */
	for (ch = 0; ch < (num_ch - 1); ch++) {
		/* Mask interrupts on channels used for data */
		err = mhu_v3_x_doorbell_mask_set(dev, ch, UINT32_MAX);
		if (err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	}

	/* Unmask doorbell notification channel interrupt */
	err = mhu_v3_x_doorbell_mask_clear(dev, (num_ch - 1), UINT32_MAX);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/*
	 * Enable the doorbell channel's contribution to mailbox combined
	 * interrupt.
	 */
	err = mhu_v3_x_channel_interrupt_enable(dev, (num_ch - 1),
			MHU_V3_X_CHANNEL_TYPE_DBCH);
	if (err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	return MHU_ERR_NONE;
}

/*
 * Public function. See mhu.h
 *
 * The basic steps of transferring a message:
 * 1. Send the size of the payload on Channel 0. It is the very first Bytes of
 *    the transfer. Continue with Channel 1.
 * 2. Send the payload, writing the channels one after the other (4 Bytes
 *    each). The last available channel is reserved for controlling the
 *    transfer. When the last channel is reached or no more data is left, STOP.
 * 3. Notify the receiver using the last channel and wait for acknowledge. If
 *    there is still data to transfer, jump to step 2. Otherwise, proceed.
 *
 */
enum mhu_error_t mhu_send_data(const uint8_t *send_buffer, size_t size)
{
	enum mhu_error_t mhu_err;
	enum mhu_v3_x_error_t mhu_v3_err;
	uint8_t num_channels;
	uint8_t chan;
	uint32_t *buffer;
	struct mhu_v3_x_dev_t *dev;

	if (size == 0) {
		return MHU_ERR_NONE;
	}

	dev = (struct mhu_v3_x_dev_t *)&mhu_hse_dev;
	chan = 0;

	if ((dev == NULL) || (dev->base == 0)) {
		return MHU_ERR_INVALID_ARG;
	}

	mhu_err = validate_buffer_params((uintptr_t)send_buffer);
	if (mhu_err != MHU_ERR_NONE) {
		return mhu_err;
	}

	mhu_v3_err = mhu_v3_x_get_num_channel_implemented(dev,
			MHU_V3_X_CHANNEL_TYPE_DBCH, &num_channels);
	if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(mhu_v3_err);
	}

	/* First send the size of the actual message. */
	mhu_v3_err = mhu_v3_x_doorbell_write(dev, chan, (uint32_t)size);
	if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(mhu_v3_err);
	}
	chan++;

	buffer = (uint32_t *)send_buffer;
	for (size_t i = 0; i < size; i += 4) {
		mhu_v3_err = mhu_v3_x_doorbell_write(dev, chan, *buffer++);
		if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(mhu_v3_err);
		}

		if (++chan == (num_channels - 1)) {
			/* Use the last channel to notify transfer complete */
			mhu_err = signal_and_wait_for_clear(
				dev, MHU_NOTIFY_VALUE);
			if (mhu_err != MHU_ERR_NONE) {
				return mhu_err;
			}
			chan = 0;
		}
	}

	if (chan != 0) {
		/* Use the last channel to notify transfer complete */
		mhu_err = signal_and_wait_for_clear(dev, MHU_NOTIFY_VALUE);
		if (mhu_err != MHU_ERR_NONE) {
			return mhu_err;
		}
	}

	return MHU_ERR_NONE;
}

/*
 * Public function. See mhu.h
 *
 * The basic steps of receiving a message:
 * 1. Read the size of the payload from Channel 0. It is the very first
 *    4 Bytes of the transfer. Continue with Channel 1.
 * 2. Receive the payload, read the channels one after the other
 *    (4 Bytes each). The last available channel is reserved for controlling
 *    the transfer.
 *    When the last channel is reached clear all the channels
 *    (also sending an acknowledge on the last channel).
 * 3. If there is still data to receive wait for a notification on the last
 *    channel and jump to step 2 as soon as it arrived. Otherwise, proceed.
 *
 */
enum mhu_error_t mhu_receive_data(uint8_t *receive_buffer, size_t *size)
{
	enum mhu_error_t mhu_err;
	enum mhu_v3_x_error_t mhu_v3_err;
	uint32_t msg_len;
	uint8_t num_channels;
	uint8_t chan;
	uint32_t *buffer;
	struct mhu_v3_x_dev_t *dev;

	dev = (struct mhu_v3_x_dev_t *)&mhu_seh_dev;
	chan = 0;

	mhu_err = validate_buffer_params((uintptr_t)receive_buffer);
	if (mhu_err != MHU_ERR_NONE) {
		return mhu_err;
	}

	mhu_v3_err = mhu_v3_x_get_num_channel_implemented(dev,
			MHU_V3_X_CHANNEL_TYPE_DBCH, &num_channels);
	if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(mhu_v3_err);
	}

	/* Busy wait for incoming reply */
	mhu_err = wait_for_signal(dev, MHU_NOTIFY_VALUE);
	if (mhu_err != MHU_ERR_NONE) {
		return mhu_err;
	}

	/* The first word is the length of the actual message. */
	mhu_v3_err = mhu_v3_x_doorbell_read(dev, chan, &msg_len);
	if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(mhu_v3_err);
	}
	chan++;

	if (*size < msg_len) {
		/* Message buffer too small */
		*size = msg_len;
		return MHU_ERR_BUFFER_TOO_SMALL;
	}

	buffer = (uint32_t *)receive_buffer;
	for (size_t i = 0; i < msg_len; i += 4) {
		mhu_v3_err = mhu_v3_x_doorbell_read(dev, chan, buffer++);
		if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(mhu_v3_err);
		}

		/* Only wait for next transfer if still missing data. */
		if (++chan == (num_channels - 1) && (msg_len - i) > 4) {
			/* Busy wait for next transfer */
			mhu_err = clear_and_wait_for_signal(
				dev, MHU_NOTIFY_VALUE);
			if (mhu_err != MHU_ERR_NONE) {
				return mhu_err;
			}
			chan = 0;
		}
	}

	/* Clear all channels */
	for (uint8_t i = U(0); i < num_channels; i++) {
		mhu_v3_err = mhu_v3_x_doorbell_clear(dev, i, UINT32_MAX);
		if (mhu_v3_err != MHU_V_3_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(mhu_v3_err);
		}
	}

	*size = msg_len;

	return MHU_ERR_NONE;
}

size_t mhu_get_max_message_size(void)
{
	enum mhu_v3_x_error_t err __maybe_unused;
	uint8_t num_channels;

	err = mhu_v3_x_get_num_channel_implemented(&mhu_seh_dev,
			MHU_V3_X_CHANNEL_TYPE_DBCH, &num_channels);

	assert(err == MHU_V_3_X_ERR_NONE);
	assert(num_channels != U(0));
	/*
	 * Returns only usable size of memory. As one channel is specifically
	 * used to inform about the size of payload, discard it from available
	 * memory size.
	 */
	return (num_channels - 1) * sizeof(uint32_t);
}
