/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MBEDTLS_PSA_CRYPTO_H
#define MBEDTLS_PSA_CRYPTO_H

#define MAX_CACHED_KEYS				10

typedef struct key_cache_s {
	const char *pk_oid;			/* Store OID of the public key */
	psa_key_id_t key_id;			/* PSA key ID */
	psa_algorithm_t psa_alg;		/* PSA Algorithm associated with the key */
	psa_key_attributes_t psa_key_attr;	/* PSA key attributes associated with the key */
	bool valid;				/* Whether this cache entry is valid */
} key_cache_t;

#endif /* MBEDTLS_PSA_CRYPTO_H */
