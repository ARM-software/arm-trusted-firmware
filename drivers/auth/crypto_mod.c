/*
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>

/* Variable exported by the crypto library through REGISTER_CRYPTO_LIB() */

/*
 * The crypto module is responsible for verifying digital signatures and hashes.
 * It relies on a crypto library to perform the cryptographic operations.
 *
 * The crypto module itself does not impose any specific format on signatures,
 * signature algorithm, keys or hashes, but most cryptographic libraries will
 * take the parameters as the following DER encoded ASN.1 structures:
 *
 *     AlgorithmIdentifier ::= SEQUENCE  {
 *         algorithm        OBJECT IDENTIFIER,
 *         parameters       ANY DEFINED BY algorithm OPTIONAL
 *     }
 *
 *     DigestInfo ::= SEQUENCE {
 *         digestAlgorithm  AlgorithmIdentifier,
 *         digest           OCTET STRING
 *     }
 *
 *     SubjectPublicKeyInfo ::= SEQUENCE  {
 *         algorithm        AlgorithmIdentifier,
 *         subjectPublicKey BIT STRING
 *     }
 *
 *     SignatureAlgorithm ::= AlgorithmIdentifier
 *
 *     SignatureValue ::= BIT STRING
 */

/*
 * Perform some static checking and call the library initialization function
 */
void crypto_mod_init(void)
{
	assert(crypto_lib_desc.name != NULL);
	assert(crypto_lib_desc.init != NULL);
#if CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
	assert(crypto_lib_desc.verify_signature != NULL);
	assert(crypto_lib_desc.verify_hash != NULL);
#endif /* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

#if CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
	assert(crypto_lib_desc.calc_hash != NULL);
#endif /* CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

	/* Initialize the cryptographic library */
	crypto_lib_desc.init();
	INFO("Using crypto library '%s'\n", crypto_lib_desc.name);
}

#if CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
/*
 * Function to verify a digital signature
 *
 * Parameters:
 *
 *   data_ptr, data_len: signed data
 *   sig_ptr, sig_len: the digital signature
 *   sig_alg_ptr, sig_alg_len: the digital signature algorithm
 *   pk_ptr, pk_len: the public key
 */
int crypto_mod_verify_signature(void *data_ptr, unsigned int data_len,
				void *sig_ptr, unsigned int sig_len,
				void *sig_alg_ptr, unsigned int sig_alg_len,
				void *pk_ptr, unsigned int pk_len)
{
	assert(data_ptr != NULL);
	assert(data_len != 0);
	assert(sig_ptr != NULL);
	assert(sig_len != 0);
	assert(sig_alg_ptr != NULL);
	assert(sig_alg_len != 0);
	assert(pk_ptr != NULL);
	assert(pk_len != 0);

	return crypto_lib_desc.verify_signature(data_ptr, data_len,
						sig_ptr, sig_len,
						sig_alg_ptr, sig_alg_len,
						pk_ptr, pk_len);
}

/*
 * Verify a hash by comparison
 *
 * Parameters:
 *
 *   data_ptr, data_len: data to be hashed
 *   digest_info_ptr, digest_info_len: hash to be compared
 */
int crypto_mod_verify_hash(void *data_ptr, unsigned int data_len,
			   void *digest_info_ptr, unsigned int digest_info_len)
{
	assert(data_ptr != NULL);
	assert(data_len != 0);
	assert(digest_info_ptr != NULL);
	assert(digest_info_len != 0);

	return crypto_lib_desc.verify_hash(data_ptr, data_len,
					   digest_info_ptr, digest_info_len);
}
#endif /* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

#if CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
/*
 * Calculate a hash
 *
 * Parameters:
 *
 *   alg: message digest algorithm
 *   data_ptr, data_len: data to be hashed
 *   output: resulting hash
 */
int crypto_mod_calc_hash(enum crypto_md_algo alg, void *data_ptr,
			 unsigned int data_len,
			 unsigned char output[CRYPTO_MD_MAX_SIZE])
{
	assert(data_ptr != NULL);
	assert(data_len != 0);
	assert(output != NULL);

	return crypto_lib_desc.calc_hash(alg, data_ptr, data_len, output);
}
#endif /* CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

/*
 * Authenticated decryption of data
 *
 * Parameters:
 *
 *   dec_algo: authenticated decryption algorithm
 *   data_ptr, len: data to be decrypted (inout param)
 *   key, key_len, key_flags: symmetric decryption key
 *   iv, iv_len: initialization vector
 *   tag, tag_len: authentication tag
 */
int crypto_mod_auth_decrypt(enum crypto_dec_algo dec_algo, void *data_ptr,
			    size_t len, const void *key, unsigned int key_len,
			    unsigned int key_flags, const void *iv,
			    unsigned int iv_len, const void *tag,
			    unsigned int tag_len)
{
	assert(crypto_lib_desc.auth_decrypt != NULL);
	assert(data_ptr != NULL);
	assert(len != 0U);
	assert(key != NULL);
	assert(key_len != 0U);
	assert(iv != NULL);
	assert((iv_len != 0U) && (iv_len <= CRYPTO_MAX_IV_SIZE));
	assert(tag != NULL);
	assert((tag_len != 0U) && (tag_len <= CRYPTO_MAX_TAG_SIZE));

	return crypto_lib_desc.auth_decrypt(dec_algo, data_ptr, len, key,
					    key_len, key_flags, iv, iv_len, tag,
					    tag_len);
}
