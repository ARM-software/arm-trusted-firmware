/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <crypto_mod.h>
#include <debug.h>

/* Variable exported by the crypto library through REGISTER_CRYPTO_LIB() */
extern const crypto_lib_desc_t crypto_lib_desc;

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
	assert(crypto_lib_desc.verify_signature != NULL);
	assert(crypto_lib_desc.verify_hash != NULL);

	/* Initialize the cryptographic library */
	crypto_lib_desc.init();
	INFO("Using crypto library '%s'\n", crypto_lib_desc.name);
}

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
