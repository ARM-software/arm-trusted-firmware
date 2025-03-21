/*
 * Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/arm/rse_comms.h>
#include <psa/client.h>
#include <rse_comms_protocol.h>

/* Union as message space and reply space are never used at the same time, and this saves space as
 * we can overlap them.
 */
union __packed __attribute__((aligned(4))) rse_comms_io_buffer_t {
	struct serialized_rse_comms_msg_t msg;
	struct serialized_rse_comms_reply_t reply;
};

static uint8_t select_protocol_version(const psa_invec *in_vec, size_t in_len,
				       const psa_outvec *out_vec, size_t out_len)
{
	size_t comms_mbx_msg_size;
	size_t comms_embed_msg_min_size;
	size_t comms_embed_reply_min_size;
	size_t in_size_total = 0;
	size_t out_size_total = 0;
	size_t i;

	for (i = 0U; i < in_len; ++i) {
		in_size_total += in_vec[i].len;
	}
	for (i = 0U; i < out_len; ++i) {
		out_size_total += out_vec[i].len;
	}

	comms_mbx_msg_size = rse_mbx_get_max_message_size();

	comms_embed_msg_min_size = sizeof(struct serialized_rse_comms_header_t) +
				   sizeof(struct rse_embed_msg_t) -
				   PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE;

	comms_embed_reply_min_size = sizeof(struct serialized_rse_comms_header_t) +
				     sizeof(struct rse_embed_reply_t) -
				     PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE;

	/* Use embed if we can pack into one message and reply, else use
	 * pointer_access. The underlying mailbox transport protocol uses a
	 * single uint32_t to track the length, so the amount of data that
	 * can be in a message is 4 bytes less than rse_mbx_get_max_message_size
	 * reports.
	 *
	 * TODO tune this with real performance numbers, it's possible a
	 * pointer_access message is less performant than multiple embed
	 * messages due to ATU configuration costs to allow access to the
	 * pointers.
	 */
	if ((comms_embed_msg_min_size + in_size_total >
	     comms_mbx_msg_size - sizeof(uint32_t)) ||
	    (comms_embed_reply_min_size + out_size_total >
	     comms_mbx_msg_size - sizeof(uint32_t))) {
		return RSE_COMMS_PROTOCOL_POINTER_ACCESS;
	} else {
		return RSE_COMMS_PROTOCOL_EMBED;
	}
}

psa_status_t psa_call(psa_handle_t handle, int32_t type, const psa_invec *in_vec, size_t in_len,
		      psa_outvec *out_vec, size_t out_len)
{
	/* Declared statically to avoid using huge amounts of stack space. Maybe revisit if
	 * functions not being reentrant becomes a problem.
	 */
	static union rse_comms_io_buffer_t io_buf;
	int err;
	psa_status_t status;
	static uint8_t seq_num = 1U;
	size_t msg_size;
	size_t reply_size = sizeof(io_buf.reply);
	psa_status_t return_val;
	size_t idx;

	if (type > PSA_CALL_TYPE_MAX || type < PSA_CALL_TYPE_MIN ||
	    in_len > PSA_MAX_IOVEC   || out_len > PSA_MAX_IOVEC) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	io_buf.msg.header.seq_num = seq_num,
	/* No need to distinguish callers (currently concurrent calls are not supported). */
	io_buf.msg.header.client_id = 1U,
	io_buf.msg.header.protocol_ver = select_protocol_version(in_vec, in_len, out_vec, out_len);

	status = rse_protocol_serialize_msg(handle, type, in_vec, in_len, out_vec,
					    out_len, &io_buf.msg, &msg_size);
	if (status != PSA_SUCCESS) {
		return status;
	}

	VERBOSE("[RSE-COMMS] Sending message\n");
	VERBOSE("protocol_ver=%u\n", io_buf.msg.header.protocol_ver);
	VERBOSE("seq_num=%u\n", io_buf.msg.header.seq_num);
	VERBOSE("client_id=%u\n", io_buf.msg.header.client_id);
	for (idx = 0; idx < in_len; idx++) {
		VERBOSE("in_vec[%lu].len=%lu\n", idx, in_vec[idx].len);
		VERBOSE("in_vec[%lu].buf=%p\n", idx, (void *)in_vec[idx].base);
	}

	err = rse_mbx_send_data((uint8_t *)&io_buf.msg, msg_size);
	if (err != 0) {
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

#if DEBUG
	/*
	 * Poisoning the message buffer (with a known pattern).
	 * Helps in detecting hypothetical RSE communication bugs.
	 */
	memset(&io_buf.msg, 0xA5, msg_size);
#endif

	err = rse_mbx_receive_data((uint8_t *)&io_buf.reply, &reply_size);
	if (err != 0) {
		return PSA_ERROR_COMMUNICATION_FAILURE;
	}

	VERBOSE("[RSE-COMMS] Received reply\n");
	VERBOSE("protocol_ver=%u\n", io_buf.reply.header.protocol_ver);
	VERBOSE("seq_num=%u\n", io_buf.reply.header.seq_num);
	VERBOSE("client_id=%u\n", io_buf.reply.header.client_id);

	status = rse_protocol_deserialize_reply(out_vec, out_len, &return_val,
						&io_buf.reply, reply_size);
	if (status != PSA_SUCCESS) {
		return status;
	}

	VERBOSE("return_val=%d\n", return_val);
	for (idx = 0U; idx < out_len; idx++) {
		VERBOSE("out_vec[%lu].len=%lu\n", idx, out_vec[idx].len);
		VERBOSE("out_vec[%lu].buf=%p\n", idx, (void *)out_vec[idx].base);
	}

	/* Clear the mailbox message buffer to remove assets from memory */
	memset(&io_buf, 0x0, sizeof(io_buf));

	seq_num++;

	return return_val;
}
