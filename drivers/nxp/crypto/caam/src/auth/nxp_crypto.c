/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <string.h>

#include "caam.h"
#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>

#include "hash.h"
#include "rsa.h"

#define LIB_NAME		"NXP crypto"

/*
 * Initialize the library and export the descriptor
 */
static void init(void)
{
	/* Initialize NXP crypto library`:*/
	NOTICE("Initializing & configuring SEC block.\n");

	if (config_sec_block() < 0) {
		ERROR("Init & config failure for caam.\n");
	}
}

/*
 * Verify a signature.
 *
 * For IMG_PLAT - data points to a PKCS#1.5 encoded HASH
 * sig_alg will be RSA or ECC
 * Parameters are passed using the DER encoding format following the ASN.1
 * structures detailed above.
 */
static int verify_signature(void *data_ptr, unsigned int data_len,
			    void *sig_ptr, unsigned int sig_len,
			    void *sign_alg, unsigned int sig_alg_len,
			    void *pk_ptr, unsigned int pk_len)
{
	int ret = CRYPTO_SUCCESS;

	enum sig_alg alg = *(enum sig_alg *)sign_alg;

	switch (alg) {
	case RSA:
		NOTICE("Verifying RSA\n");
		ret = rsa_verify_signature(data_ptr, data_len, sig_ptr, sig_len,
					   pk_ptr, pk_len);
		break;
	case ECC:
	default:
		ret = CRYPTO_ERR_SIGNATURE;
		break;
	}

	if (ret != 0) {
		ERROR("RSA verification Failed\n");
	}
	return ret;

}

/*
 * Match a hash
 *
 * Digest info is passed as a table of SHA-26 hashes and digest_info_len
 * is number of entries in the table
 * This implementation is very specific to the CSF header parser ROTPK
 * comparison.
 */
static int verify_hash(void *data_ptr, unsigned int data_len,
		       void *digest_info_ptr, unsigned int digest_info_len)
{
	void *ctx = NULL;
	int i = 0, ret = 0;
	enum hash_algo algo = SHA256;
	uint8_t hash[SHA256_BYTES] __aligned(CACHE_WRITEBACK_GRANULE) = {0};
	uint32_t digest_size = SHA256_BYTES;
	uint8_t *hash_tbl = digest_info_ptr;

	NOTICE("Verifying hash\n");
	ret = hash_init(algo, &ctx);
	if (ret != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Update hash with that of SRK table */
	ret = hash_update(algo, ctx, data_ptr, data_len);
	if (ret != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Copy hash at destination buffer */
	ret = hash_final(algo, ctx, hash, digest_size);
	if (ret != 0) {
		return CRYPTO_ERR_HASH;
	}

	VERBOSE("%s Calculated hash\n", __func__);
	for (i = 0; i < SHA256_BYTES/4; i++) {
		VERBOSE("%x\n", *((uint32_t *)hash + i));
	}

	for (i = 0; i < digest_info_len; i++) {
		if (memcmp(hash, (hash_tbl + (i * digest_size)),
			   digest_size) == 0) {
			return CRYPTO_SUCCESS;
		}
	}

	return CRYPTO_ERR_HASH;
}

/*
 * Register crypto library descriptor
 */
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, NULL);
