/*
 * Copyright (c) 2024, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMM_EL3_TOKEN_SIGN_H
#define RMM_EL3_TOKEN_SIGN_H

#include <stdint.h>
#include <lib/cassert.h>
#include <services/rmmd_svc.h>

/*
 * Defines member of structure and reserves space
 * for the next member with specified offset.
 */
/* cppcheck-suppress [misra-c2012-20.7] */
#define SET_MEMBER(member, start, end)	\
	union {				\
		member;			\
		unsigned char reserved##end[((end) - (start))];	\
	}

#define EL3_TOKEN_RESPONSE_MAX_SIG_LEN U(512)

struct el3_token_sign_request {
	SET_MEMBER(uint32_t sig_alg_id, 0x0, 0x8);
	SET_MEMBER(uint64_t rec_granule, 0x8, 0x10);
	SET_MEMBER(uint64_t req_ticket, 0x10, 0x18);
	SET_MEMBER(uint32_t hash_alg_id, 0x18, 0x20);
	SET_MEMBER(uint8_t hash_buf[SHA512_DIGEST_SIZE], 0x20, 0x60);
};

CASSERT(__builtin_offsetof(struct el3_token_sign_request, sig_alg_id) == 0x0U,
	assert_el3_token_sign_request_sig_alg_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_request, rec_granule) == 0x8U,
	assert_el3_token_sign_request_rec_granule_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_request, req_ticket) == 0x10U,
	assert_el3_token_sign_request_req_ticket_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_request, hash_alg_id) == 0x18U,
	assert_el3_token_sign_request_hash_alg_id_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_request, hash_buf) == 0x20U,
	assert_el3_token_sign_request_hash_buf_mismatch);


struct el3_token_sign_response {
	SET_MEMBER(uint64_t rec_granule, 0x0, 0x8);
	SET_MEMBER(uint64_t req_ticket, 0x8, 0x10);
	SET_MEMBER(uint16_t sig_len, 0x10, 0x12);
	SET_MEMBER(uint8_t signature_buf[EL3_TOKEN_RESPONSE_MAX_SIG_LEN], 0x12, 0x212);
};

CASSERT(__builtin_offsetof(struct el3_token_sign_response, rec_granule) == 0x0U,
	assert_el3_token_sign_resp_rec_granule_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_response, req_ticket) == 0x8U,
	assert_el3_token_sign_resp_req_ticket_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_response, sig_len) == 0x10U,
	assert_el3_token_sign_resp_sig_len_mismatch);
CASSERT(__builtin_offsetof(struct el3_token_sign_response, signature_buf) == 0x12U,
	assert_el3_token_sign_resp_sig_buf_mismatch);

#endif /* RMM_EL3_TOKEN_SIGN_H */
