/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
