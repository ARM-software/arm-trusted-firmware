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


#include <crypto_mod.h>
#include <debug.h>
#include <mbedtls_common.h>
#include <stddef.h>
#include <string.h>

/* mbedTLS headers */
#include <polarssl/md_wrap.h>
#include <polarssl/memory_buffer_alloc.h>
#include <polarssl/oid.h>
#include <polarssl/platform.h>

#define LIB_NAME		"mbedTLS"

/*
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm               OBJECT IDENTIFIER,
 *     parameters              ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * SubjectPublicKeyInfo  ::=  SEQUENCE  {
 *     algorithm            AlgorithmIdentifier,
 *     subjectPublicKey     BIT STRING
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm AlgorithmIdentifier,
 *     digest OCTET STRING
 * }
 */

/*
 * Initialize the library and export the descriptor
 */
static void init(void)
{
	/* Initialize mbedTLS */
	mbedtls_init();
}

/*
 * Verify a signature.
 *
 * Parameters are passed using the DER encoding format following the ASN.1
 * structures detailed above.
 */
static int verify_signature(void *data_ptr, unsigned int data_len,
			    void *sig_ptr, unsigned int sig_len,
			    void *sig_alg, unsigned int sig_alg_len,
			    void *pk_ptr, unsigned int pk_len)
{
	asn1_buf sig_oid, sig_params;
	asn1_buf signature;
	md_type_t md_alg;
	pk_type_t pk_alg;
	pk_context pk;
	int rc;
	void *sig_opts = NULL;
	const md_info_t *md_info;
	unsigned char *p, *end;
	unsigned char hash[POLARSSL_MD_MAX_SIZE];

	/* Get pointers to signature OID and parameters */
	p = (unsigned char *)sig_alg;
	end = (unsigned char *)(p + sig_alg_len);
	rc = asn1_get_alg(&p, end, &sig_oid, &sig_params);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	/* Get the actual signature algorithm (MD + PK) */
	rc = oid_get_sig_alg(&sig_oid, &md_alg, &pk_alg);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	/* Parse the public key */
	pk_init(&pk);
	p = (unsigned char *)pk_ptr;
	end = (unsigned char *)(p + pk_len);
	rc = pk_parse_subpubkey(&p, end, &pk);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	/* Get the signature (bitstring) */
	p = (unsigned char *)sig_ptr;
	end = (unsigned char *)(p + sig_len);
	signature.tag = *p;
	rc = asn1_get_bitstring_null(&p, end, &signature.len);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end;
	}
	signature.p = p;

	/* Calculate the hash of the data */
	md_info = md_info_from_type(md_alg);
	if (md_info == NULL) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end;
	}
	p = (unsigned char *)data_ptr;
	rc = md(md_info, p, data_len, hash);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end;
	}

	/* Verify the signature */
	rc = pk_verify_ext(pk_alg, sig_opts, &pk, md_alg, hash,
			md_info->size, signature.p, signature.len);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end;
	}

	/* Signature verification success */
	rc = CRYPTO_SUCCESS;

end:
	pk_free(&pk);
	return rc;
}

/*
 * Match a hash
 *
 * Digest info is passed in DER format following the ASN.1 structure detailed
 * above.
 */
static int verify_hash(void *data_ptr, unsigned int data_len,
		       void *digest_info_ptr, unsigned int digest_info_len)
{
	asn1_buf hash_oid, params;
	md_type_t md_alg;
	const md_info_t *md_info;
	unsigned char *p, *end, *hash;
	unsigned char data_hash[POLARSSL_MD_MAX_SIZE];
	size_t len;
	int rc;

	/* Digest info should be an ASN1_SEQUENCE */
	p = (unsigned char *)digest_info_ptr;
	end = (unsigned char *)(digest_info_ptr + digest_info_len);
	rc = asn1_get_tag(&p, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Get the hash algorithm */
	rc = asn1_get_alg(&p, end, &hash_oid, &params);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	rc = oid_get_md_alg(&hash_oid, &md_alg);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	md_info = md_info_from_type(md_alg);
	if (md_info == NULL) {
		return CRYPTO_ERR_HASH;
	}

	/* Hash should be octet string type */
	rc = asn1_get_tag(&p, end, &len, ASN1_OCTET_STRING);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Length of hash must match the algorithm's size */
	if (len != md_info->size) {
		return CRYPTO_ERR_HASH;
	}
	hash = p;

	/* Calculate the hash of the data */
	p = (unsigned char *)data_ptr;
	rc = md(md_info, p, data_len, data_hash);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Compare values */
	rc = memcmp(data_hash, hash, md_info->size);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	return CRYPTO_SUCCESS;
}

/*
 * Register crypto library descriptor
 */
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash);
