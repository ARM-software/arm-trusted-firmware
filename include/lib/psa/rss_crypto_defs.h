/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef RSS_CRYPTO_DEFS_H
#define RSS_CRYPTO_DEFS_H

/* Declares types that encode errors, algorithms, key types, policies, etc. */
#include "psa/crypto_types.h"

/*
 * Value identifying export public key function API, used to dispatch the request
 * to the corresponding API implementation in the Crypto service backend.
 *
 */
#define RSS_CRYPTO_EXPORT_PUBLIC_KEY_SID	(uint16_t)(0x701)

/*
 * The persistent key identifiers for RSS builtin keys.
 */
enum rss_key_id_builtin_t {
	RSS_BUILTIN_KEY_ID_HOST_S_ROTPK = 0x7FFF816Cu,
	RSS_BUILTIN_KEY_ID_HOST_NS_ROTPK,
	RSS_BUILTIN_KEY_ID_HOST_CCA_ROTPK,
};

/*
 * This type is used to overcome a limitation within RSS firmware in the number of maximum
 * IOVECs it can use especially in psa_aead_encrypt and psa_aead_decrypt.
 */
#define RSS_CRYPTO_MAX_NONCE_LENGTH (16u)
struct rss_crypto_aead_pack_input {
	uint8_t nonce[RSS_CRYPTO_MAX_NONCE_LENGTH];
	uint32_t nonce_length;
};

/*
 * Structure used to pack non-pointer types in a call
 */
struct rss_crypto_pack_iovec {
	psa_key_id_t key_id;	/* Key id */
	psa_algorithm_t alg;	/* Algorithm */
	uint32_t op_handle;	/* Frontend context handle associated
				   to a multipart operation */
	uint32_t capacity;	/* Key derivation capacity */
	uint32_t ad_length;	/* Additional Data length for multipart AEAD */
	uint32_t plaintext_length;	/* Plaintext length for multipart AEAD */
	struct rss_crypto_aead_pack_input aead_in;	/* Packs AEAD-related inputs */
	uint16_t function_id;	/* Used to identify the function in the API dispatcher
				   to the service backend. See rss_crypto_func_sid for
				   detail */
	uint16_t step;		/* Key derivation step */
};

#endif /* RSS_CRYPTO_DEFS_H */
