/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_INITIAL_ATTESTATION_H
#define PSA_INITIAL_ATTESTATION_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "psa/error.h"

/*
 * Initial attestation API version is: 1.0.0
 */
#define PSA_INITIAL_ATTEST_API_VERSION_MAJOR	(1)
#define PSA_INITIAL_ATTEST_API_VERSION_MINOR	(0)

/* The allowed size of input challenge in bytes. */
#define PSA_INITIAL_ATTEST_CHALLENGE_SIZE_32	32U
#define PSA_INITIAL_ATTEST_CHALLENGE_SIZE_48	48U
#define PSA_INITIAL_ATTEST_CHALLENGE_SIZE_64	64U

/* Initial Attestation message types that distinguish Attest services. */
#define RSS_ATTEST_GET_TOKEN		1001U
#define RSS_ATTEST_GET_TOKEN_SIZE	1002U
#define RSS_ATTEST_GET_DELEGATED_KEY	1003U

/**
 * Get the platform attestation token.
 *
 * auth_challenge	Pointer to buffer where challenge input is stored. This
 *			must be the hash of the public part of the delegated
 *			attestation key.
 * challenge_size	Size of challenge object in bytes.
 * token_buf		Pointer to the buffer where attestation token will be
 *			stored.
 * token_buf_size	Size of allocated buffer for token, in bytes.
 * token_size		Size of the token that has been returned, in bytes.
 *
 * Returns error code as specified in psa_status_t.
 */
psa_status_t
psa_initial_attest_get_token(const uint8_t *auth_challenge,
			     size_t         challenge_size,
			     uint8_t       *token_buf,
			     size_t         token_buf_size,
			     size_t        *token_size);

#endif /* PSA_INITIAL_ATTESTATION_H */
