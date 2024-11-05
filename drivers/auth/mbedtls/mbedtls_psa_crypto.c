/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

/* mbed TLS headers */
#include <mbedtls/md.h>
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/oid.h>
#include <mbedtls/platform.h>
#include <mbedtls/psa_util.h>
#include <mbedtls/x509.h>
#include <psa/crypto.h>
#include <psa/crypto_platform.h>
#include <psa/crypto_types.h>
#include <psa/crypto_values.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/auth/mbedtls/mbedtls_common.h>
#include <plat/common/platform.h>

#define LIB_NAME		"mbed TLS PSA"

/* Minimum required size for a buffer containing a raw EC signature when using
 * a maximum curve size of 384 bits.
 * This is calculated as 2 * (384 / 8). */
#define ECDSA_SIG_BUFFER_SIZE	96U

/* Size of ASN.1 length and tag in bytes*/
#define SIZE_OF_ASN1_LEN	1U
#define SIZE_OF_ASN1_TAG	1U

#if CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
/*
 * CRYPTO_MD_MAX_SIZE value is as per current stronger algorithm available
 * so make sure that mbed TLS MD maximum size must be lesser than this.
 */
CASSERT(CRYPTO_MD_MAX_SIZE >= MBEDTLS_MD_MAX_SIZE,
	assert_mbedtls_md_size_overflow);

#endif /*
	* CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
	* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
	*/

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
 * We pretend using an external RNG (through MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG
 * mbedTLS config option) so we need to provide an implementation of
 * mbedtls_psa_external_get_random(). Provide a fake one, since we do not
 * actually have any external RNG and TF-A itself doesn't engage in
 * cryptographic operations that demands randomness.
 */
psa_status_t mbedtls_psa_external_get_random(
			mbedtls_psa_external_random_context_t *context,
			uint8_t *output, size_t output_size,
			size_t *output_length)
{
	return PSA_ERROR_INSUFFICIENT_ENTROPY;
}

/*
 * Initialize the library and export the descriptor
 */
static void init(void)
{
	/* Initialize mbed TLS */
	mbedtls_init();

	/* Initialise PSA mbedTLS */
	psa_status_t status = psa_crypto_init();

	if (status != PSA_SUCCESS) {
		ERROR("Failed to initialize %s crypto (%d).\n", LIB_NAME, status);
		panic();
	}

	INFO("PSA crypto initialized successfully!\n");
}

#if CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC

/*
 * NOTE: This has been made internal in mbedtls 3.6.0 and the mbedtls team has
 * advised that it's better to copy out the declaration than it would be to
 * update to 3.5.2, where this function is exposed.
 */
int mbedtls_x509_get_sig_alg(const mbedtls_x509_buf *sig_oid,
			     const mbedtls_x509_buf *sig_params,
			     mbedtls_md_type_t *md_alg,
			     mbedtls_pk_type_t *pk_alg,
			     void **sig_opts);

/*
 * This is a helper function which parses a SignatureAlgorithm OID.
 * It extracts the pk algorithm and constructs a psa_algorithm_t object
 * to be used by PSA calls.
 */
static int construct_psa_alg(void *sig_alg, unsigned int sig_alg_len,
			     mbedtls_pk_type_t *pk_alg, psa_algorithm_t *psa_alg)
{
	int rc;
	mbedtls_md_type_t md_alg;
	void *sig_opts = NULL;
	mbedtls_asn1_buf sig_alg_oid, params;
	unsigned char *p = (unsigned char *) sig_alg;
	unsigned char *end = (unsigned char *) sig_alg + sig_alg_len;

	rc = mbedtls_asn1_get_alg(&p, end, &sig_alg_oid, &params);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end;
	}

	rc = mbedtls_x509_get_sig_alg(&sig_alg_oid, &params, &md_alg, pk_alg, &sig_opts);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end;
	}

	psa_algorithm_t psa_md_alg = mbedtls_md_psa_alg_from_type(md_alg);

	switch (*pk_alg) {
	case MBEDTLS_PK_RSASSA_PSS:
		*psa_alg = PSA_ALG_RSA_PSS(psa_md_alg);
		rc = CRYPTO_SUCCESS;
		break;
	case MBEDTLS_PK_ECDSA:
		*psa_alg = PSA_ALG_ECDSA(psa_md_alg);
		rc = CRYPTO_SUCCESS;
		break;
	default:
		*psa_alg = PSA_ALG_NONE;
		rc = CRYPTO_ERR_SIGNATURE;
		break;
	}

end:
	mbedtls_free(sig_opts);
	return rc;
}

/*
 * Helper functions for mbedtls PK contexts.
 */
static void initialize_pk_context(mbedtls_pk_context *pk, bool *pk_initialized)
{
	mbedtls_pk_init(pk);
	*pk_initialized = true;
}

static void cleanup_pk_context(mbedtls_pk_context *pk, bool *pk_initialized)
{
	if (*pk_initialized) {
		mbedtls_pk_free(pk);
		*pk_initialized = false;
	}
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
	unsigned char *p, *end;
	mbedtls_pk_context pk;
	bool pk_initialized = false;
	int rc = CRYPTO_ERR_SIGNATURE;
	psa_status_t psa_status = PSA_ERROR_CORRUPTION_DETECTED;
	psa_key_attributes_t psa_key_attr = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t psa_key_id;
	mbedtls_pk_type_t pk_alg;
	psa_algorithm_t psa_alg;
	__unused unsigned char reformatted_sig[ECDSA_SIG_BUFFER_SIZE] = {0};
	unsigned char *local_sig_ptr;
	size_t local_sig_len;

	/* Load the key into the PSA key store. */
	initialize_pk_context(&pk, &pk_initialized);

	p = (unsigned char *) pk_ptr;
	end = p + pk_len;
	rc = mbedtls_pk_parse_subpubkey(&p, end, &pk);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	rc = mbedtls_pk_get_psa_attributes(&pk, PSA_KEY_USAGE_VERIFY_MESSAGE, &psa_key_attr);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	rc = construct_psa_alg(sig_alg, sig_alg_len, &pk_alg, &psa_alg);
	if (rc != CRYPTO_SUCCESS) {
		goto end2;
	}
	psa_set_key_algorithm(&psa_key_attr, psa_alg);

	rc = mbedtls_pk_import_into_psa(&pk, &psa_key_attr, &psa_key_id);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	/* Optimize mbedtls heap usage by freeing the pk context now.  */
	cleanup_pk_context(&pk, &pk_initialized);

	/* Extract the signature from sig_ptr. */
	p = (unsigned char *) sig_ptr;
	end = p + sig_len;
	rc = mbedtls_asn1_get_bitstring_null(&p, end, &local_sig_len);
	if (rc != 0) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end1;
	}
	local_sig_ptr = p;

#if TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA || \
TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
	if (pk_alg == MBEDTLS_PK_ECDSA) {
		/* Convert the DER ASN.1 signature to raw format. */
		size_t key_bits = psa_get_key_bits(&psa_key_attr);

		rc = mbedtls_ecdsa_der_to_raw(key_bits, p, local_sig_len,
					      reformatted_sig, ECDSA_SIG_BUFFER_SIZE,
					      &local_sig_len);
		if (rc != 0) {
			rc = CRYPTO_ERR_SIGNATURE;
			goto end1;
		}
		local_sig_ptr = reformatted_sig;
	}
#endif /*
	* TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA || \
	* TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
	**/

	/* Verify the signature. */
	psa_status = psa_verify_message(psa_key_id, psa_alg,
				    data_ptr, data_len,
				    local_sig_ptr, local_sig_len);
	if (psa_status == PSA_SUCCESS) {
		/* The signature has been successfully verified. */
		rc = CRYPTO_SUCCESS;
	} else {
		rc = CRYPTO_ERR_SIGNATURE;
	}

end1:
	/* Destroy the key from the PSA subsystem. */
	psa_destroy_key(psa_key_id);
end2:
	/* Free the pk context, if it is initialized. */
	cleanup_pk_context(&pk, &pk_initialized);

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
	unsigned char *p, *end, *hash;
	size_t len;
	int rc;
	psa_status_t status;
	psa_algorithm_t psa_md_alg;

	/*
	 * Digest info should be an MBEDTLS_ASN1_SEQUENCE, but padding after
	 * it is allowed.  This is necessary to support multiple hash
	 * algorithms.
	 */
	p = (unsigned char *)digest_info_ptr;
	end = p + digest_info_len;
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				  MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	end = p + len;

	/* Get the hash algorithm */
	rc = mbedtls_asn1_get_alg(&p, end, &hash_oid, &params);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* Hash should be octet string type and consume all bytes */
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
	if ((rc != 0) || ((size_t)(end - p) != len)) {
		return CRYPTO_ERR_HASH;
	}
	hash = p;

	rc = mbedtls_oid_get_md_alg(&hash_oid, &md_alg);
	if (rc != 0) {
		return CRYPTO_ERR_HASH;
	}

	/* convert the md_alg to psa_algo */
	psa_md_alg = mbedtls_md_psa_alg_from_type(md_alg);

	/* Length of hash must match the algorithm's size */
	if (len != PSA_HASH_LENGTH(psa_md_alg)) {
		return CRYPTO_ERR_HASH;
	}

	/*
	 * Calculate Hash and compare it against the retrieved hash from
	 * the certificate (one shot API).
	 */
	status = psa_hash_compare(psa_md_alg,
				  data_ptr, (size_t)data_len,
				  (const uint8_t *)hash, len);

	if (status != PSA_SUCCESS) {
		return CRYPTO_ERR_HASH;
	}

	return CRYPTO_SUCCESS;
}
#endif /*
	* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
	* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
	*/

#if CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
/*
 * Map a generic crypto message digest algorithm to the corresponding macro used
 * by Mbed TLS.
 */
static inline mbedtls_md_type_t md_type(enum crypto_md_algo algo)
{
	switch (algo) {
	case CRYPTO_MD_SHA512:
		return MBEDTLS_MD_SHA512;
	case CRYPTO_MD_SHA384:
		return MBEDTLS_MD_SHA384;
	case CRYPTO_MD_SHA256:
		return MBEDTLS_MD_SHA256;
	default:
		/* Invalid hash algorithm. */
		return MBEDTLS_MD_NONE;
	}
}

/*
 * Calculate a hash
 *
 * output points to the computed hash
 */
static int calc_hash(enum crypto_md_algo md_algo, void *data_ptr,
		     unsigned int data_len,
		     unsigned char output[CRYPTO_MD_MAX_SIZE])
{
	size_t hash_length;
	psa_status_t status;
	psa_algorithm_t psa_md_alg;

	/* convert the md_alg to psa_algo */
	psa_md_alg = mbedtls_md_psa_alg_from_type(md_type(md_algo));

	/*
	 * Calculate the hash of the data, it is safe to pass the
	 * 'output' hash buffer pointer considering its size is always
	 * bigger than or equal to MBEDTLS_MD_MAX_SIZE.
	 */
	status = psa_hash_compute(psa_md_alg, data_ptr, (size_t)data_len,
				  (uint8_t *)output, CRYPTO_MD_MAX_SIZE,
				  &hash_length);
	if (status != PSA_SUCCESS) {
		return CRYPTO_ERR_HASH;
	}

	return CRYPTO_SUCCESS;
}
#endif /*
	* CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
	* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
	*/

#if TF_MBEDTLS_USE_AES_GCM
/*
 * Stack based buffer allocation for decryption operation. It could
 * be configured to balance stack usage vs execution speed.
 */
#define DEC_OP_BUF_SIZE		128

static int aes_gcm_decrypt(void *data_ptr, size_t len, const void *key,
			   unsigned int key_len, const void *iv,
			   unsigned int iv_len, const void *tag,
			   unsigned int tag_len)
{
	mbedtls_svc_key_id_t key_id = MBEDTLS_SVC_KEY_ID_INIT;
	psa_aead_operation_t operation = PSA_AEAD_OPERATION_INIT;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	psa_status_t psa_status = PSA_ERROR_GENERIC_ERROR;
	unsigned char buf[DEC_OP_BUF_SIZE];
	unsigned char *pt = data_ptr;
	size_t dec_len;
	size_t output_length;

	/* Load the key into the PSA key store. */
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DECRYPT);
	psa_set_key_algorithm(&attributes, PSA_ALG_GCM);
	psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);

	psa_status = psa_import_key(&attributes, key, key_len, &key_id);
	if (psa_status != PSA_SUCCESS) {
		return CRYPTO_ERR_DECRYPTION;
	}

	/* Perform the decryption. */
	psa_status = psa_aead_decrypt_setup(&operation, key_id, PSA_ALG_GCM);
	if (psa_status != PSA_SUCCESS) {
		goto err;
	}

	psa_status = psa_aead_set_nonce(&operation, iv, iv_len);
	if (psa_status != PSA_SUCCESS) {
		goto err;
	}

	while (len > 0) {
		dec_len = MIN(sizeof(buf), len);

		psa_status = psa_aead_update(&operation, pt, dec_len, buf,
					     sizeof(buf), &output_length);
		if (psa_status != PSA_SUCCESS) {
			goto err;
		}

		memcpy(pt, buf, output_length);
		pt += output_length;
		len -= dec_len;
	}

	/* Verify the tag. */
	psa_status = psa_aead_verify(&operation, NULL, 0, &output_length, tag, tag_len);
	if (psa_status == PSA_SUCCESS) {
		psa_destroy_key(key_id);
		return CRYPTO_SUCCESS;
	}

err:
	psa_aead_abort(&operation);
	psa_destroy_key(key_id);
	return CRYPTO_ERR_DECRYPTION;
}

/*
 * Authenticated decryption of an image
 */
static int auth_decrypt(enum crypto_dec_algo dec_algo, void *data_ptr,
			size_t len, const void *key, unsigned int key_len,
			unsigned int key_flags, const void *iv,
			unsigned int iv_len, const void *tag,
			unsigned int tag_len)
{
	int rc;

	assert((key_flags & ENC_KEY_IS_IDENTIFIER) == 0);

	switch (dec_algo) {
	case CRYPTO_GCM_DECRYPT:
		rc = aes_gcm_decrypt(data_ptr, len, key, key_len, iv, iv_len,
				     tag, tag_len);
		if (rc != 0)
			return rc;
		break;
	default:
		return CRYPTO_ERR_DECRYPTION;
	}

	return CRYPTO_SUCCESS;
}
#endif /* TF_MBEDTLS_USE_AES_GCM */

/*
 * Register crypto library descriptor
 */
#if CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
#if TF_MBEDTLS_USE_AES_GCM
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, calc_hash,
		    auth_decrypt, NULL);
#else
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, calc_hash,
		    NULL, NULL);
#endif
#elif CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY
#if TF_MBEDTLS_USE_AES_GCM
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, NULL,
		    auth_decrypt, NULL);
#else
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, NULL,
		    NULL, NULL);
#endif
#elif CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY
REGISTER_CRYPTO_LIB(LIB_NAME, init, NULL, NULL, calc_hash, NULL, NULL);
#endif /* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */
