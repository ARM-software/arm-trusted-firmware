/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <drivers/arm/mhu.h>

#include "mhu_v2_x.h"

#define MHU_NOTIFY_VALUE	(1234u)

/*
 * MHU devices for host:
 * HSE: Host to Secure Enclave (sender device)
 * SEH: Secure Enclave to Host (receiver device)
 */
struct mhu_v2_x_dev_t MHU1_HSE_DEV = {0, MHU_V2_X_SENDER_FRAME};
struct mhu_v2_x_dev_t MHU1_SEH_DEV = {0, MHU_V2_X_RECEIVER_FRAME};

static enum mhu_error_t error_mapping_to_mhu_error_t(enum mhu_v2_x_error_t err)
{
	switch (err) {
	case MHU_V_2_X_ERR_NONE:
		return MHU_ERR_NONE;
	case MHU_V_2_X_ERR_NOT_INIT:
		return MHU_ERR_NOT_INIT;
	case MHU_V_2_X_ERR_ALREADY_INIT:
		return MHU_ERR_ALREADY_INIT;
	case MHU_V_2_X_ERR_UNSUPPORTED_VERSION:
		return MHU_ERR_UNSUPPORTED_VERSION;
	case MHU_V_2_X_ERR_INVALID_ARG:
		return MHU_ERR_INVALID_ARG;
	case MHU_V_2_X_ERR_GENERAL:
		return MHU_ERR_GENERAL;
	default:
		return MHU_ERR_GENERAL;
	}
}

static enum mhu_v2_x_error_t signal_and_wait_for_clear(void)
{
	enum mhu_v2_x_error_t err;
	struct mhu_v2_x_dev_t *dev = &MHU1_HSE_DEV;
	uint32_t val = MHU_NOTIFY_VALUE;
	/* Using the last channel for notifications */
	uint32_t channel_notify = mhu_v2_x_get_num_channel_implemented(dev) - 1;

	err = mhu_v2_x_channel_send(dev, channel_notify, val);
	if (err != MHU_V_2_X_ERR_NONE) {
		return err;
	}

	do {
		err = mhu_v2_x_channel_poll(dev, channel_notify, &val);
		if (err != MHU_V_2_X_ERR_NONE) {
			break;
		}
	} while (val != 0);

	return err;
}

static enum mhu_v2_x_error_t wait_for_signal(void)
{
	enum mhu_v2_x_error_t err;
	struct mhu_v2_x_dev_t *dev = &MHU1_SEH_DEV;
	uint32_t val = 0;
	/* Using the last channel for notifications */
	uint32_t channel_notify = mhu_v2_x_get_num_channel_implemented(dev) - 1;

	do {
		err = mhu_v2_x_channel_receive(dev, channel_notify, &val);
		if (err != MHU_V_2_X_ERR_NONE) {
			break;
		}
	} while (val != MHU_NOTIFY_VALUE);

	return err;
}

static enum mhu_v2_x_error_t clear_and_wait_for_next_signal(void)
{
	enum mhu_v2_x_error_t err;
	struct mhu_v2_x_dev_t *dev = &MHU1_SEH_DEV;
	uint32_t num_channels = mhu_v2_x_get_num_channel_implemented(dev);
	uint32_t i;

	/* Clear all channels */
	for (i = 0; i < num_channels; ++i) {
		err = mhu_v2_x_channel_clear(dev, i);
		if (err != MHU_V_2_X_ERR_NONE) {
			return err;
		}
	}

	return wait_for_signal();
}

enum mhu_error_t mhu_init_sender(uintptr_t mhu_sender_base)
{
	enum mhu_v2_x_error_t err;

	assert(mhu_sender_base != (uintptr_t)NULL);

	MHU1_HSE_DEV.base = mhu_sender_base;

	err = mhu_v2_x_driver_init(&MHU1_HSE_DEV, MHU_REV_READ_FROM_HW);
	return error_mapping_to_mhu_error_t(err);
}

enum mhu_error_t mhu_init_receiver(uintptr_t mhu_receiver_base)
{
	enum mhu_v2_x_error_t err;
	uint32_t num_channels, i;

	assert(mhu_receiver_base != (uintptr_t)NULL);

	MHU1_SEH_DEV.base = mhu_receiver_base;

	err = mhu_v2_x_driver_init(&MHU1_SEH_DEV, MHU_REV_READ_FROM_HW);
	if (err != MHU_V_2_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	num_channels = mhu_v2_x_get_num_channel_implemented(&MHU1_SEH_DEV);

	/* Mask all channels except the notifying channel */
	for (i = 0; i < (num_channels - 1); ++i) {
		err = mhu_v2_x_channel_mask_set(&MHU1_SEH_DEV, i, UINT32_MAX);
		if (err != MHU_V_2_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	}

	/* The last channel is used for notifications */
	err = mhu_v2_x_channel_mask_clear(
		&MHU1_SEH_DEV, (num_channels - 1), UINT32_MAX);
	return error_mapping_to_mhu_error_t(err);
}

/*
 * Public function. See mhu.h
 *
 * The basic steps of transferring a message:
 * 1.	Initiate MHU transfer.
 * 2.	Send over the size of the payload on Channel 1. It is the very first
 *	4 Bytes of the transfer. Continue with Channel 2.
 * 3.	Send over the payload, writing the channels one after the other
 *	(4 Bytes each). The last available channel is reserved for controlling
 *	the transfer.
 *	When the last channel is reached or no more data is left, STOP.
 * 4.	Notify the receiver using the last channel and wait for acknowledge.
 *	If there is still data to transfer, jump to step 3. Otherwise, proceed.
 * 5.	Close MHU transfer.
 *
 */
enum mhu_error_t mhu_send_data(const uint8_t *send_buffer, size_t size)
{
	enum mhu_v2_x_error_t err;
	struct mhu_v2_x_dev_t *dev = &MHU1_HSE_DEV;
	uint32_t num_channels = mhu_v2_x_get_num_channel_implemented(dev);
	uint32_t chan = 0;
	uint32_t i;
	uint32_t *p;

	/* For simplicity, require the send_buffer to be 4-byte aligned */
	if ((uintptr_t)send_buffer & 0x3U) {
		return MHU_ERR_INVALID_ARG;
	}

	err = mhu_v2_x_initiate_transfer(dev);
	if (err != MHU_V_2_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* First send over the size of the actual message */
	err = mhu_v2_x_channel_send(dev, chan, (uint32_t)size);
	if (err != MHU_V_2_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}
	chan++;

	p = (uint32_t *)send_buffer;
	for (i = 0; i < size; i += 4) {
		err = mhu_v2_x_channel_send(dev, chan, *p++);
		if (err != MHU_V_2_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
		if (++chan == (num_channels - 1)) {
			err = signal_and_wait_for_clear();
			if (err != MHU_V_2_X_ERR_NONE) {
				return error_mapping_to_mhu_error_t(err);
			}
			chan = 0;
		}
	}

	/* Signal the end of transfer.
	 *   It's not required to send a signal when the message was
	 *   perfectly-aligned (num_channels - 1 channels were used in the last
	 *   round) preventing it from signaling twice at the end of transfer.
	 */
	if (chan != 0) {
		err = signal_and_wait_for_clear();
		if (err != MHU_V_2_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	}

	err = mhu_v2_x_close_transfer(dev);
	return error_mapping_to_mhu_error_t(err);
}

/*
 * Public function. See mhu.h
 *
 * The basic steps of receiving a message:
 * 1.	Read the size of the payload from Channel 1. It is the very first
 *	4 Bytes of the transfer. Continue with Channel 2.
 * 2.	Receive the payload, read the channels one after the other
 *	(4 Bytes each). The last available channel is reserved for controlling
 *	the transfer.
 *	When the last channel is reached clear all the channels
 *	(also sending an acknowledge on the last channel).
 * 3.	If there is still data to receive wait for a notification on the last
 *	channel and jump to step 2 as soon as it arrived. Otherwise, proceed.
 * 4.	End of transfer.
 *
 */
enum mhu_error_t mhu_receive_data(uint8_t *receive_buffer, size_t *size)
{
	enum mhu_v2_x_error_t err;
	struct mhu_v2_x_dev_t *dev = &MHU1_SEH_DEV;
	uint32_t num_channels = mhu_v2_x_get_num_channel_implemented(dev);
	uint32_t chan = 0;
	uint32_t message_len;
	uint32_t i;
	uint32_t *p;

	/* For simplicity, require:
	 * - the receive_buffer to be 4-byte aligned,
	 * - the buffer size to be a multiple of 4.
	 */
	if (((uintptr_t)receive_buffer & 0x3U) || (*size & 0x3U)) {
		return MHU_ERR_INVALID_ARG;
	}

	/* Busy wait for incoming reply */
	err = wait_for_signal();
	if (err != MHU_V_2_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}

	/* The first word is the length of the actual message */
	err = mhu_v2_x_channel_receive(dev, chan, &message_len);
	if (err != MHU_V_2_X_ERR_NONE) {
		return error_mapping_to_mhu_error_t(err);
	}
	chan++;

	if (message_len > *size) {
		/* Message buffer too small */
		*size = message_len;
		return MHU_ERR_BUFFER_TOO_SMALL;
	}

	p = (uint32_t *)receive_buffer;
	for (i = 0; i < message_len; i += 4) {
		err = mhu_v2_x_channel_receive(dev, chan, p++);
		if (err != MHU_V_2_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}

		/* Only wait for next transfer if there is still missing data */
		if (++chan == (num_channels - 1) && (message_len - i) > 4) {
			/* Busy wait for next transfer */
			err = clear_and_wait_for_next_signal();
			if (err != MHU_V_2_X_ERR_NONE) {
				return error_mapping_to_mhu_error_t(err);
			}
			chan = 0;
		}
	}

	/* Clear all channels */
	for (i = 0; i < num_channels; ++i) {
		err = mhu_v2_x_channel_clear(dev, i);
		if (err != MHU_V_2_X_ERR_NONE) {
			return error_mapping_to_mhu_error_t(err);
		}
	}

	*size = message_len;

	return MHU_ERR_NONE;
}
