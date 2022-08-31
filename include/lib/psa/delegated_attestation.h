/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* This file describes the Delegated Attestation API */

#ifndef DELEGATED_ATTESTATION_H
#define DELEGATED_ATTESTATION_H

#include <stddef.h>
#include <stdint.h>

#include "psa/error.h"

/* RSS Delegated Attestation message types that distinguish its services. */
#define RSS_DELEGATED_ATTEST_GET_DELEGATED_KEY      1001U
#define RSS_DELEGATED_ATTEST_GET_PLATFORM_TOKEN     1002U

/**
 * The aim of these APIs to get a derived signing key (private only) for the
 * delegated attestation model and obtain the corresponding platform attestation
 * token. In the delegated attestation model the final token consist of more
 * than one subtokens which are signed by different entities. There is a
 * cryptographical binding between the tokens. The derived delegated attestation
 * key is bind to the platform token (details below).
 *
 * Expected usage model:
 *  - First rss_delegated_attest_get_delegated_key() API need to be called to
 *    obtain the private part of the delegated attestation key. The public part
 *    of key is computed by the cryptographic library when the key is
 *    registered.
 *  - Secondly the rss_delegated_attest_get_token() must be called to obtain
 *    platform attestation token. The hash of the public key (computed by
 *    the hash_algo indicated in the rss_delegated_attest_get_delegated_key()
 *    call) must be the input of this call. This ensures that nothing but the
 *    previously derived delegated key is bindable to the platform token.
 */

/**
 * Get a delegated attestation key (DAK).
 *
 * The aim of the delegated attestation key is to enable other SW components
 * within the system to sign an attestation token which is different than the
 * initial/platform token. The initial attestation token MUST contain the hash
 * of the public delegated key to make a cryptographical binding (hash lock)
 * between the key and the token.
 * The initial attestation token has two roles in this scenario:
 *   - Attest the device boot status and security lifecycle.
 *   - Attest the delegated attestation key.
 * The delegated attestation key is derived from a preprovisioned seed. The
 * input for the key derivation is the platform boot status. The system can be
 * attestated with the two tokens together.
 *
 * ecc_curve     The type of the elliptic curve to which the requested
 *               attestation key belongs. Please check the note section for
 *               limitations.
 * key_bits      The size of the requested attestation key, in bits.
 * key_buf       Pointer to the buffer where the delegated attestation key will
 *               be stored.
 * key_buf_size  Size of allocated buffer for the key, in bytes.
 * key_size      Size of the key that has been returned, in bytes.
 * hash_algo     The hash algorithm that will be used later by the owner of the
 *               requested delegated key for binding it to the platform
 *               attestation token.
 *
 * Returns error code as specified in psa_status_t.
 *
 * Notes:
 *   - Currently, only the PSA_ECC_FAMILY_SECP_R1 curve type is supported.
 *   - The delegated attestation key must be derived before requesting for the
 *     platform attestation token as they are cryptographically linked together.
 */
psa_status_t
rss_delegated_attest_get_delegated_key(uint8_t   ecc_curve,
				       uint32_t  key_bits,
				       uint8_t  *key_buf,
				       size_t    key_buf_size,
				       size_t   *key_size,
				       uint32_t  hash_algo);

/**
 * Get platform attestation token
 *
 * dak_pub_hash       Pointer to buffer where the hash of the public DAK is
 *                    stored.
 * dak_pub_hash_size  Size of the hash value, in bytes.
 * token_buf          Pointer to the buffer where the platform attestation token
 *                    will be stored.
 * token_buf_size     Size of allocated buffer for token, in bytes.
 * token_size         Size of the token that has been returned, in bytes.
 *
 * Returns error code as specified in psa_status_t.
 *
 * A delegated attestation key must be derived before requesting for the
 * platform attestation token as they are cryptographically linked together.
 * Otherwise, the token request will fail and the PSA_ERROR_INVALID_ARGUMENT
 * code will be returned.
 */
psa_status_t
rss_delegated_attest_get_token(const uint8_t *dak_pub_hash,
			       size_t         dak_pub_hash_size,
			       uint8_t       *token_buf,
			       size_t         token_buf_size,
			       size_t        *token_size);

#endif /* DELEGATED_ATTESTATION_H */
