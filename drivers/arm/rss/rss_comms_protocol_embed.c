/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include "rss_comms_protocol_embed.h"

#define TYPE_OFFSET	(16U)
#define TYPE_MASK	(0xFFFFUL << TYPE_OFFSET)
#define IN_LEN_OFFSET	(8U)
#define IN_LEN_MASK	(0xFFUL << IN_LEN_OFFSET)
#define OUT_LEN_OFFSET	(0U)
#define OUT_LEN_MASK	(0xFFUL << OUT_LEN_OFFSET)

#define PARAM_PACK(type, in_len, out_len)			  \
	(((((uint32_t)type) << TYPE_OFFSET) & TYPE_MASK)	| \
	 ((((uint32_t)in_len) << IN_LEN_OFFSET) & IN_LEN_MASK)	| \
	 ((((uint32_t)out_len) << OUT_LEN_OFFSET) & OUT_LEN_MASK))

psa_status_t rss_protocol_embed_serialize_msg(psa_handle_t handle,
					      int16_t type,
					      const psa_invec *in_vec,
					      uint8_t in_len,
					      const psa_outvec *out_vec,
					      uint8_t out_len,
					      struct rss_embed_msg_t *msg,
					      size_t *msg_len)
{
	uint32_t payload_size = 0;
	uint32_t i;

	assert(msg != NULL);
	assert(msg_len != NULL);
	assert(in_vec != NULL);

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
		memcpy(msg->trailer + payload_size, in_vec[i].base, in_vec[i].len);
		payload_size += in_vec[i].len;
	}

	/* Output the actual size of the message, to optimize sending */
	*msg_len = sizeof(*msg) - sizeof(msg->trailer) + payload_size;

	return PSA_SUCCESS;
}

psa_status_t rss_protocol_embed_deserialize_reply(psa_outvec *out_vec,
						  uint8_t out_len,
						  psa_status_t *return_val,
						  const struct rss_embed_reply_t *reply,
						  size_t reply_size)
{
	uint32_t payload_offset = 0;
	uint32_t i;

	assert(reply != NULL);
	assert(return_val != NULL);

	for (i = 0U; i < out_len; ++i) {
		if (sizeof(reply) - sizeof(reply->trailer) + payload_offset > reply_size) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}

		memcpy(out_vec[i].base, reply->trailer + payload_offset, out_vec[i].len);
		payload_offset += out_vec[i].len;
	}

	*return_val = reply->return_val;

	return PSA_SUCCESS;
}
