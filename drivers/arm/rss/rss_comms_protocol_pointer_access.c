/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <assert.h>

#include "rss_comms_protocol_pointer_access.h"

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

psa_status_t rss_protocol_pointer_access_serialize_msg(psa_handle_t handle,
						       int16_t type,
						       const psa_invec *in_vec,
						       uint8_t in_len,
						       const psa_outvec *out_vec,
						       uint8_t out_len,
						       struct rss_pointer_access_msg_t *msg,
						       size_t *msg_len)
{
	unsigned int i;

	assert(msg != NULL);
	assert(msg_len != NULL);
	assert(in_vec != NULL);

	msg->ctrl_param = PARAM_PACK(type, in_len, out_len);
	msg->handle = handle;

	/* Fill msg iovec lengths */
	for (i = 0U; i < in_len; ++i) {
		msg->io_sizes[i] = in_vec[i].len;
		msg->host_ptrs[i] = (uint64_t)in_vec[i].base;
	}
	for (i = 0U; i < out_len; ++i) {
		msg->io_sizes[in_len + i] = out_vec[i].len;
		msg->host_ptrs[in_len + i] = (uint64_t)out_vec[i].base;
	}

	*msg_len = sizeof(*msg);

	return PSA_SUCCESS;
}

psa_status_t rss_protocol_pointer_access_deserialize_reply(psa_outvec *out_vec,
							   uint8_t out_len,
							   psa_status_t *return_val,
							   const struct rss_pointer_access_reply_t *reply,
							   size_t reply_size)
{
	unsigned int i;

	assert(reply != NULL);
	assert(return_val != NULL);

	for (i = 0U; i < out_len; ++i) {
		out_vec[i].len = reply->out_sizes[i];
	}

	*return_val = reply->return_val;

	return PSA_SUCCESS;
}
