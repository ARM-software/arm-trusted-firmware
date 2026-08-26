/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include "rse_comms_protocol_common.h"
#include "rse_comms_protocol_embed.h"

psa_status_t rse_protocol_embed_serialize_msg(psa_handle_t handle,
					      int16_t type,
					      const psa_invec *in_vec,
					      uint8_t in_len,
					      const psa_outvec *out_vec,
					      uint8_t out_len,
					      struct rse_embed_msg_t *msg,
					      size_t *msg_len)
{
	uint32_t payload_size = 0;
	uint32_t i;

	assert(msg != NULL);
	assert(msg_len != NULL);
	assert((in_len == 0u) || (in_vec != NULL));
	assert((out_len == 0u) || (out_vec != NULL));

	msg->ctrl_param = PARAM_PACK(type, in_len, out_len);
	msg->handle = handle;

	/* Fill msg iovec lengths */
	for (i = 0U; i < in_len; ++i) {
		msg->io_size[i] = in_vec[i].len;
	}
	for (i = 0U; i < out_len; ++i) {
		msg->io_size[in_len + i] = out_vec[i].len;
	}

	for (i = 0U; i < in_len; ++i) {
		if (in_vec[i].len > sizeof(msg->trailer) - payload_size) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}
		memcpy(msg->trailer + payload_size,
		       in_vec[i].base,
		       in_vec[i].len);
		payload_size += in_vec[i].len;
	}

	/* Output the actual size of the message, to optimize sending */
	*msg_len = sizeof(*msg) - sizeof(msg->trailer) + payload_size;

	return PSA_SUCCESS;
}

psa_status_t rse_protocol_embed_deserialize_reply(psa_outvec *out_vec,
						  uint8_t out_len,
						  psa_status_t *return_val,
						  const struct rse_embed_reply_t *reply,
						  size_t reply_size)
{
	size_t header_size = sizeof(*reply) - sizeof(reply->trailer);
	size_t payload_offset = 0U;
	size_t payload_size;
	size_t output_size;
	uint32_t i;

	assert(reply != NULL);
	assert(return_val != NULL);
	assert((out_len == 0U) || (out_vec != NULL));

	if (reply_size < header_size) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	payload_size = reply_size - header_size;

	/* Validate the complete reply before copying any output data. */
	for (i = 0U; i < out_len; ++i) {
		output_size = reply->out_size[i];
		if ((output_size > out_vec[i].len) ||
		    (output_size > payload_size - payload_offset)) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}

		payload_offset += output_size;
	}

	payload_offset = 0U;
	for (i = 0U; i < out_len; ++i) {
		output_size = reply->out_size[i];
		memcpy(out_vec[i].base, reply->trailer + payload_offset,
		       output_size);
		out_vec[i].len = output_size;
		payload_offset += output_size;
	}

	*return_val = reply->return_val;

	return PSA_SUCCESS;
}
