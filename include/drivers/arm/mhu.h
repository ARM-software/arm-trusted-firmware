/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MHU_H
#define MHU_H

#include <stddef.h>
#include <stdint.h>

/**
 * Generic MHU error enumeration types.
 */
enum mhu_error_t {
	MHU_ERR_NONE			=  0,
	MHU_ERR_NOT_INIT		= -1,
	MHU_ERR_ALREADY_INIT		= -2,
	MHU_ERR_UNSUPPORTED_VERSION	= -3,
	MHU_ERR_UNSUPPORTED		= -4,
	MHU_ERR_INVALID_ARG		= -5,
	MHU_ERR_BUFFER_TOO_SMALL	= -6,
	MHU_ERR_GENERAL			= -7,
};

/**
 * Structure used by RSE comms
 */
struct mhu_addr {
	uintptr_t sender_base;
	uintptr_t receiver_base;
};

/**
 * Initializes sender MHU.
 *
 * mhu_sender_base	Base address of sender MHU.
 *
 * Returns mhu_error_t error code.
 *
 * This function must be called before mhu_send_data().
 */
enum mhu_error_t mhu_init_sender(uintptr_t mhu_sender_base);


/**
 * Initializes receiver MHU.
 *
 * mhu_receiver_base	Base address of receiver MHU.
 *
 * Returns mhu_error_t error code.
 *
 * This function must be called before mhu_receive_data().
 */
enum mhu_error_t mhu_init_receiver(uintptr_t mhu_receiver_base);

/**
 * Sends data over MHU.
 *
 * send_buffer		Pointer to buffer containing the data to be transmitted.
 * size			Size of the data to be transmitted in bytes.
 *
 * Returns mhu_error_t error code.
 *
 * The send_buffer must be 4-byte aligned and its length must be at least
 * (4 - (size % 4)) bytes bigger than the data size to prevent buffer
 * over-reading.
 */
enum mhu_error_t mhu_send_data(const uint8_t *send_buffer, size_t size);

/**
 * Receives data from MHU.
 *
 * receive_buffer	Pointer the buffer where to store the received data.
 * size			As input the size of the receive_buffer, as output the
 *			number of bytes received. As a limitation,
 *			the size of the buffer must be a multiple of 4.
 *
 * Returns mhu_error_t error code.
 *
 * The receive_buffer must be 4-byte aligned and its length must be a
 * multiple of 4.
 */
enum mhu_error_t mhu_receive_data(uint8_t *receive_buffer, size_t *size);

/**
 * Gets the maximum amount of bytes that can be transmitted in a single send by MHU.
 *
 * Returns The amount of bytes that can be sent or received in a single message.
 */
size_t mhu_get_max_message_size(void);

#endif /* MHU_H */
