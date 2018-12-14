/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>

/* mbed TLS headers */
#include <mbedtls/md.h>
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/oid.h>
#include <mbedtls/platform.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/auth/mbedtls/mbedtls_common.h>
#include <drivers/auth/mbedtls/mbedtls_config.h>

#define LIB_NAME		"mbed TLS"

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
	/* Initialize mbed TLS */
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
	mbedtls_asn1_buf sig_oid, sig_params;
	mbedtls_asn1_buf signature;
	mbedtls_md_type_t md_alg;
	mbedtls_pk_type_t pk_alg;
	mbedtls_pk_context pk = {0};
	int rc;
	void *sig_opts = NULL;
	const mbedtls_md_info_t *md_info;
	unsigned char *p, *end;
	unsigned char hash[MBEDTLS_MD_MAX_SIZE];

	/* Get pointers to signature OID and parameters */
	p = (unsigned char *)sig_alg;
	end = (unsigned char *)(p + sig_alg_len);
	rc = mbedtls_asn1_get_alg(&p, end, &sig_oid, &sig_params);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	/* Get the actual signature algorithm (MD + PK) */
	rc = mbedtls_x509_get_sig_alg(&sig_oid, &sig_params, &md_alg, &pk_alg, &sig_opts);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	/* Parse the public key */
	mbedtls_pk_init(&pk);
	p = (unsigned char *)pk_ptr;
	end = (unsigned char *)(p + pk_len);
	rc = mbedtls_pk_parse_subpubkey(&p, end, &pk);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	/* Get the signature (bitstring) */
	p = (unsigned char *)sig_ptr;
	end = (unsigned char *)(p + sig_len);
	signature.tag = *p;
	rc = mbedtls_asn1_get_bitstring_null(&p, end, &signature.len);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end1;
	}
	signature.p = p;

	/* Calculate the hash of the data */
	md_info = mbedtls_md_info_from_type(md_alg);
	if (md_info == NULL) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end1;
	}
	p = (unsigned char *)data_ptr;
	rc = mbedtls_md(md_info, p, data_len, hash);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end1;
	}

	/* Verify the signature */
	rc = mbedtls_pk_verify_ext(pk_alg, sig_opts, &pk, md_alg, hash,
			mbedtls_md_get_size(md_info),
			signature.p, signature.len);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end1;
	}

	/* Signature verification success */
	rc = CRYPTO_SUCCESS;

end1:
	mbedtls_pk_free(&pk);
end2:
	mbedtls_free(sig_opts);
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
	mbedtls_asn1_buf hash_oid, params;
	mbedtls_md_type_t md_alg;
	const mbedtls_md_info_t *md_info;
	unsigned char *p, *end, *hash;
	unsigned char data_hash[MBEDTLS_MD_MAX_SIZE];
	size_t len;
	int rc;

	/* Digest info should be an MBEDTLS_ASN1_SEQUENCE */
	p = (unsigned char *)digest_info_ptr;
	end = p + digest_info_len;
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				  MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Get the hash algorithm */
	rc = mbedtls_asn1_get_alg(&p, end, &hash_oid, &params);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	rc = mbedtls_oid_get_md_alg(&hash_oid, &md_alg);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	md_info = mbedtls_md_info_from_type(md_alg);
	if (md_info == NULL) {
		return CRYPTO_ERR_HASH;
	}

	/* Hash should be octet string type */
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Length of hash must match the algorithm's size */
	if (len != mbedtls_md_get_size(md_info)) {
		return CRYPTO_ERR_HASH;
	}
	hash = p;

	/* Calculate the hash of the data */
	p = (unsigned char *)data_ptr;
	rc = mbedtls_md(md_info, p, data_len, data_hash);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Compare values */
	rc = memcmp(data_hash, hash, mbedtls_md_get_size(md_info));
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	return CRYPTO_SUCCESS;
}

/*
 * Register crypto library descriptor
 */
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash);
