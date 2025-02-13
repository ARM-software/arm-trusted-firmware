/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/arm/mhu.h>
#include <drivers/arm/rse_comms.h>

size_t rse_mbx_get_max_message_size(void)
{
	return mhu_get_max_message_size();
}

int rse_mbx_send_data(const uint8_t *send_buffer, size_t size)
{
	enum mhu_error_t err = mhu_send_data(send_buffer, size);

	if (err != MHU_ERR_NONE) {
		ERROR("mhu_send_data err=%d\n", err);
		return -1;
	}

	return 0;
}

int rse_mbx_receive_data(uint8_t *receive_buffer, size_t *size)
{
	enum mhu_error_t err = mhu_receive_data(receive_buffer, size);

	if (err != MHU_ERR_NONE) {
		ERROR("mhu_receive_data err=%d\n", err);
		return -1;
	}

	return 0;
}

int rse_mbx_init(const void *init_data)
{
	enum mhu_error_t err;
	const struct mhu_addr *mbx_addr = (const struct mhu_addr *)init_data;

	err = mhu_init_sender(mbx_addr->sender_base);
	if (err != MHU_ERR_NONE) {
		if (err == MHU_ERR_ALREADY_INIT) {
			INFO("[RSE-COMMS] Host to RSE MHU driver already initialized\n");
		} else {
			ERROR("[RSE-COMMS] Host to RSE MHU driver initialization failed: %d\n",
			      err);
			return -1;
		}
	}

	err = mhu_init_receiver(mbx_addr->receiver_base);
	if (err != MHU_ERR_NONE) {
		if (err == MHU_ERR_ALREADY_INIT) {
			INFO("[RSE-COMMS] RSE to Host MHU driver already initialized\n");
		} else {
			ERROR("[RSE-COMMS] RSE to Host MHU driver initialization failed: %d\n",
			      err);
			return -1;
		}
	}

	return 0;
}
