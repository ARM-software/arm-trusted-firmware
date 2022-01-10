/*
 * Copyright (c) 2020-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MHU_V2_X_H
#define MHU_V2_X_H

#include <stdbool.h>
#include <stdint.h>

#define MHU_2_X_INTR_NR2R_OFF		(0x0u)
#define MHU_2_X_INTR_R2NR_OFF		(0x1u)
#define MHU_2_1_INTR_CHCOMB_OFF		(0x2u)

#define MHU_2_X_INTR_NR2R_MASK		(0x1u << MHU_2_X_INTR_NR2R_OFF)
#define MHU_2_X_INTR_R2NR_MASK		(0x1u << MHU_2_X_INTR_R2NR_OFF)
#define MHU_2_1_INTR_CHCOMB_MASK	(0x1u << MHU_2_1_INTR_CHCOMB_OFF)

enum mhu_v2_x_frame_t {
	MHU_V2_X_SENDER_FRAME   = 0x0u,
	MHU_V2_X_RECEIVER_FRAME = 0x1u,
};

enum mhu_v2_x_supported_revisions {
	MHU_REV_READ_FROM_HW = 0,
	MHU_REV_2_0,
	MHU_REV_2_1,
};

struct mhu_v2_x_dev_t {
	uintptr_t base;
	enum mhu_v2_x_frame_t frame;
	uint32_t subversion;	/*!< Hardware subversion: v2.X */
	bool is_initialized;	/*!< Indicates if the MHU driver
				 *   is initialized and enabled
				 */
};

/**
 * MHU v2 error enumeration types.
 */
enum mhu_v2_x_error_t {
	MHU_V_2_X_ERR_NONE			=  0,
	MHU_V_2_X_ERR_NOT_INIT			= -1,
	MHU_V_2_X_ERR_ALREADY_INIT		= -2,
	MHU_V_2_X_ERR_UNSUPPORTED_VERSION	= -3,
	MHU_V_2_X_ERR_INVALID_ARG		= -4,
	MHU_V_2_X_ERR_GENERAL			= -5
};

/**
 * Initializes the driver.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 * rev		MHU revision (if can't be identified from HW).
 *
 * Reads the MHU hardware version.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * MHU revision only has to be specified when versions can't be read
 * from HW (ARCH_MAJOR_REV reg reads as 0x0).
 *
 * This function doesn't check if dev is NULL.
 */
enum mhu_v2_x_error_t mhu_v2_x_driver_init(struct mhu_v2_x_dev_t *dev,
	enum mhu_v2_x_supported_revisions rev);

/**
 * Returns the number of channels implemented.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 *
 * This function doesn't check if dev is NULL.
 */
uint32_t mhu_v2_x_get_num_channel_implemented(
		const struct mhu_v2_x_dev_t *dev);

/**
 * Sends the value over a channel.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 * channel	Channel to send the value over.
 * val		Value to send.
 *
 * Sends the value over a channel.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * This function doesn't check if dev is NULL.
 * This function doesn't check if channel is implemented.
 */
enum mhu_v2_x_error_t mhu_v2_x_channel_send(const struct mhu_v2_x_dev_t *dev,
	uint32_t channel, uint32_t val);

/**
 * Polls sender channel status.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 * channel	Channel to poll the status of.
 * value	Pointer to variable that will store the value.
 *
 * Polls sender channel status.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * This function doesn't check if dev is NULL.
 * This function doesn't check if channel is implemented.
 */
enum mhu_v2_x_error_t mhu_v2_x_channel_poll(const struct mhu_v2_x_dev_t *dev,
	uint32_t channel, uint32_t *value);

/**
 * Clears the channel after the value is send over it.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 * channel	Channel to clear.
 *
 * Clears the channel after the value is send over it.
 *
 * Returns mhu_v2_x_error_t error code..
 *
 * This function doesn't check if dev is NULL.
 * This function doesn't check if channel is implemented.
 */
enum mhu_v2_x_error_t mhu_v2_x_channel_clear(const struct mhu_v2_x_dev_t *dev,
	uint32_t channel);

/**
 * Receives the value over a channel.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 * channel	Channel to receive the value from.
 * value	Pointer to variable that will store the value.
 *
 * Receives the value over a channel.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * This function doesn't check if dev is NULL.
 * This function doesn't check if channel is implemented.
 */
enum mhu_v2_x_error_t mhu_v2_x_channel_receive(
	const struct mhu_v2_x_dev_t *dev, uint32_t channel, uint32_t *value);

/**
 * Sets bits in the Channel Mask.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 * channel	Which channel's mask to set.
 * mask		Mask to be set over a receiver frame.
 *
 * Sets bits in the Channel Mask.
 *
 * Returns mhu_v2_x_error_t error code..
 *
 * This function doesn't check if dev is NULL.
 *  This function doesn't check if channel is implemented.
 */
enum mhu_v2_x_error_t mhu_v2_x_channel_mask_set(
	const struct mhu_v2_x_dev_t *dev, uint32_t channel, uint32_t mask);

/**
 * Clears bits in the Channel Mask.
 *
 * dev	MHU device struct mhu_v2_x_dev_t.
 * channel	Which channel's mask to clear.
 * mask	Mask to be clear over a receiver frame.
 *
 * Clears bits in the Channel Mask.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * This function doesn't check if dev is NULL.
 *  This function doesn't check if channel is implemented.
 */
enum mhu_v2_x_error_t mhu_v2_x_channel_mask_clear(
	const struct mhu_v2_x_dev_t *dev, uint32_t channel, uint32_t mask);

/**
 * Initiates a MHU transfer with the handshake signals.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 *
 * Initiates a MHU transfer with the handshake signals in a blocking mode.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * This function doesn't check if dev is NULL.
 */
enum mhu_v2_x_error_t mhu_v2_x_initiate_transfer(
	const struct mhu_v2_x_dev_t *dev);

/**
 * Closes a MHU transfer with the handshake signals.
 *
 * dev		MHU device struct mhu_v2_x_dev_t.
 *
 * Closes a MHU transfer with the handshake signals in a blocking mode.
 *
 * Returns mhu_v2_x_error_t error code.
 *
 * This function doesn't check if dev is NULL.
 */
enum mhu_v2_x_error_t mhu_v2_x_close_transfer(
	const struct mhu_v2_x_dev_t *dev);

#endif /* MHU_V2_X_H */
