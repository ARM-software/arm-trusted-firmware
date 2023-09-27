/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

/* mbed TLS headers */
#include <mbedtls/gcm.h>
#include <mbedtls/md.h>
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/oid.h>
#include <mbedtls/platform.h>
#include <mbedtls/version.h>
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

/* Maximum length of R_S pair in the ECDSA signature in bytes */
#define MAX_ECDSA_R_S_PAIR_LEN	64U

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

static inline psa_algorithm_t mbedtls_md_psa_alg_from_type(
						mbedtls_md_type_t md_type)
{
	assert((md_type == MBEDTLS_MD_SHA256) ||
	       (md_type == MBEDTLS_MD_SHA384) ||
	       (md_type == MBEDTLS_MD_SHA512));

	return PSA_ALG_CATEGORY_HASH | (psa_algorithm_t) (md_type + 0x5);
}

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

static void construct_psa_key_alg_and_type(mbedtls_pk_type_t pk_alg,
					   mbedtls_md_type_t md_alg,
					   psa_ecc_family_t psa_ecc_family,
					   psa_algorithm_t *psa_alg,
					   psa_key_type_t *psa_key_type)
{
	psa_algorithm_t psa_md_alg = mbedtls_md_psa_alg_from_type(md_alg);

	switch (pk_alg) {
	case MBEDTLS_PK_RSASSA_PSS:
		*psa_alg = PSA_ALG_RSA_PSS(psa_md_alg);
		*psa_key_type = PSA_KEY_TYPE_RSA_PUBLIC_KEY;
		break;
	case MBEDTLS_PK_ECDSA:
		*psa_alg = PSA_ALG_ECDSA(psa_md_alg);
		*psa_key_type = PSA_KEY_TYPE_ECC_PUBLIC_KEY(psa_ecc_family);
		break;
	default:
		*psa_alg = PSA_ALG_NONE;
		*psa_key_type = PSA_KEY_TYPE_NONE;
		break;
	}
}


#if TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA || \
TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA

/*
 * This is a helper function to detect padding byte (if the MSB bit of the
 * first data byte is set to 1, for example 0x80) and on detection, ignore the
 * padded byte(0x00) and increase the buffer pointer beyond padded byte and
 * decrease the length of the buffer by 1.
 *
 * On Success returns 0, error otherwise.
 **/
static inline int ignore_asn1_int_padding_byte(unsigned char **buf_start,
					       size_t *buf_len)
{
	unsigned char *local_buf = *buf_start;

	/* Check for negative number */
	if ((local_buf[0] & 0x80U) != 0U) {
		return -1;
	}

	if ((local_buf[0] == 0U) && (local_buf[1] > 0x7FU) &&
	    (*buf_len > 1U)) {
		*buf_start = &local_buf[1];
		(*buf_len)--;
	}

	return 0;
}

/*
 * This is a helper function that gets a pointer to the encoded ECDSA publicKey
 * and its length (as per RFC5280) and returns corresponding decoded publicKey
 * and its length. As well, it retrieves the family of ECC key in the PSA
 * format.
 *
 * This function returns error(CRYPTO_ERR_SIGNATURE) on ASN.1 parsing failure,
 * otherwise success(0).
 **/
static int get_ecdsa_pkinfo_from_asn1(unsigned char **pk_start,
				      unsigned int *pk_len,
				      psa_ecc_family_t *psa_ecc_family)
{
	mbedtls_asn1_buf alg_oid, alg_params;
	mbedtls_ecp_group_id grp_id;
	int rc;
	unsigned char *pk_end;
	size_t len;
	size_t curve_bits;
	unsigned char *pk_ptr = *pk_start;

	pk_end = pk_ptr + *pk_len;
	rc = mbedtls_asn1_get_tag(&pk_ptr, pk_end, &len,
				  MBEDTLS_ASN1_CONSTRUCTED |
				  MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	pk_end = pk_ptr + len;
	rc = mbedtls_asn1_get_alg(&pk_ptr, pk_end, &alg_oid, &alg_params);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	if (alg_params.tag == MBEDTLS_ASN1_OID) {
		if (mbedtls_oid_get_ec_grp(&alg_params, &grp_id) != 0) {
			return CRYPTO_ERR_SIGNATURE;
		}
		*psa_ecc_family = mbedtls_ecc_group_to_psa(grp_id,
							   &curve_bits);
	} else {
		return CRYPTO_ERR_SIGNATURE;
	}

	pk_end = pk_ptr + len - (alg_oid.len + alg_params.len +
		 2 * (SIZE_OF_ASN1_LEN + SIZE_OF_ASN1_TAG));
	rc = mbedtls_asn1_get_bitstring_null(&pk_ptr, pk_end, &len);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	*pk_start = pk_ptr;
	*pk_len = len;

	return rc;
}

/*
 * Ecdsa-Sig-Value  ::=  SEQUENCE  {
 *   r     INTEGER,
 *   s     INTEGER
 * }
 *
 * This helper function that gets a pointer to the encoded ECDSA signature and
 * its length (as per RFC5280) and returns corresponding decoded signature
 * (R_S pair) and its size.
 *
 * This function returns error(CRYPTO_ERR_SIGNATURE) on ASN.1 parsing failure,
 * otherwise success(0).
 **/
static int get_ecdsa_signature_from_asn1(unsigned char *sig_ptr,
					 size_t *sig_len,
					 unsigned char *r_s_pair)
{
	int rc;
	unsigned char *sig_end;
	size_t len, r_len, s_len;

	sig_end = sig_ptr + *sig_len;
	rc = mbedtls_asn1_get_tag(&sig_ptr, sig_end, &len,
				  MBEDTLS_ASN1_CONSTRUCTED |
				  MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	sig_end = sig_ptr + len;
	rc = mbedtls_asn1_get_tag(&sig_ptr, sig_end, &r_len,
				  MBEDTLS_ASN1_INTEGER);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	if (ignore_asn1_int_padding_byte(&sig_ptr, &r_len) != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	(void)memcpy((void *)&r_s_pair[0], (const void *)sig_ptr, r_len);

	sig_ptr = sig_ptr + r_len;
	sig_end = sig_ptr + len - (r_len + (SIZE_OF_ASN1_LEN +
		  SIZE_OF_ASN1_TAG));
	rc = mbedtls_asn1_get_tag(&sig_ptr, sig_end, &s_len,
				  MBEDTLS_ASN1_INTEGER);
	if (rc != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	if (ignore_asn1_int_padding_byte(&sig_ptr, &s_len) != 0) {
		return CRYPTO_ERR_SIGNATURE;
	}

	(void)memcpy((void *)&r_s_pair[r_len], (const void *)sig_ptr, s_len);

	*sig_len = s_len + r_len;

	return 0;
}
#endif /*
	* TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA || \
	* TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
	**/

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
	int rc;
	void *sig_opts = NULL;
	unsigned char *p, *end;
	unsigned char *local_sig_ptr;
	size_t local_sig_len;
	psa_ecc_family_t psa_ecc_family = 0U;
	__unused unsigned char reformatted_sig[MAX_ECDSA_R_S_PAIR_LEN] = {0};

	/* construct PSA key algo and type */
	psa_status_t status = PSA_SUCCESS;
	psa_key_attributes_t psa_key_attr = PSA_KEY_ATTRIBUTES_INIT;
	psa_key_id_t psa_key_id = PSA_KEY_ID_NULL;
	psa_key_type_t psa_key_type;
	psa_algorithm_t psa_alg;

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

	/* Get the signature (bitstring) */
	p = (unsigned char *)sig_ptr;
	end = (unsigned char *)(p + sig_len);
	signature.tag = *p;
	rc = mbedtls_asn1_get_bitstring_null(&p, end, &signature.len);
	if ((rc != 0) || ((size_t)(end - p) != signature.len)) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	local_sig_ptr = p;
	local_sig_len = signature.len;

#if TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA || \
TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
	if (pk_alg == MBEDTLS_PK_ECDSA) {
		rc = get_ecdsa_signature_from_asn1(local_sig_ptr,
						   &local_sig_len,
						   reformatted_sig);
		if (rc != 0) {
			goto end2;
		}

		local_sig_ptr = reformatted_sig;

		rc = get_ecdsa_pkinfo_from_asn1((unsigned char **)&pk_ptr,
						&pk_len,
						&psa_ecc_family);
		if (rc != 0) {
			goto end2;
		}
	}
#endif /*
	* TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_ECDSA || \
	* TF_MBEDTLS_KEY_ALG_ID == TF_MBEDTLS_RSA_AND_ECDSA
	**/

	/* Convert this pk_alg and md_alg to PSA key type and key algorithm */
	construct_psa_key_alg_and_type(pk_alg, md_alg, psa_ecc_family,
				       &psa_alg, &psa_key_type);


	if ((psa_alg == PSA_ALG_NONE) || (psa_key_type == PSA_KEY_TYPE_NONE)) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	/* filled-in key_attributes */
	psa_set_key_algorithm(&psa_key_attr, psa_alg);
	psa_set_key_type(&psa_key_attr, psa_key_type);
	psa_set_key_usage_flags(&psa_key_attr, PSA_KEY_USAGE_VERIFY_MESSAGE);

	/* Get the key_id using import API */
	status = psa_import_key(&psa_key_attr,
				pk_ptr,
				(size_t)pk_len,
				&psa_key_id);

	if (status != PSA_SUCCESS) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end2;
	}

	/*
	 * Hash calculation and Signature verification of the given data payload
	 * is wrapped under the psa_verify_message function.
	 */
	status = psa_verify_message(psa_key_id, psa_alg,
				    data_ptr, data_len,
				    local_sig_ptr, local_sig_len);

	if (status != PSA_SUCCESS) {
		rc = CRYPTO_ERR_SIGNATURE;
		goto end1;
	}

	/* Signature verification success */
	rc = CRYPTO_SUCCESS;

end1:
	/*
	 * Destroy the key if it is created successfully
	 */
	psa_destroy_key(psa_key_id);
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
	mbedtls_gcm_context ctx;
	mbedtls_cipher_id_t cipher = MBEDTLS_CIPHER_ID_AES;
	unsigned char buf[DEC_OP_BUF_SIZE];
	unsigned char tag_buf[CRYPTO_MAX_TAG_SIZE];
	unsigned char *pt = data_ptr;
	size_t dec_len;
	int diff, i, rc;
	size_t output_length __unused;

	mbedtls_gcm_init(&ctx);

	rc = mbedtls_gcm_setkey(&ctx, cipher, key, key_len * 8);
	if (rc != 0) {
		rc = CRYPTO_ERR_DECRYPTION;
		goto exit_gcm;
	}

#if (MBEDTLS_VERSION_MAJOR < 3)
	rc = mbedtls_gcm_starts(&ctx, MBEDTLS_GCM_DECRYPT, iv, iv_len, NULL, 0);
#else
	rc = mbedtls_gcm_starts(&ctx, MBEDTLS_GCM_DECRYPT, iv, iv_len);
#endif
	if (rc != 0) {
		rc = CRYPTO_ERR_DECRYPTION;
		goto exit_gcm;
	}

	while (len > 0) {
		dec_len = MIN(sizeof(buf), len);

#if (MBEDTLS_VERSION_MAJOR < 3)
		rc = mbedtls_gcm_update(&ctx, dec_len, pt, buf);
#else
		rc = mbedtls_gcm_update(&ctx, pt, dec_len, buf, sizeof(buf), &output_length);
#endif

		if (rc != 0) {
			rc = CRYPTO_ERR_DECRYPTION;
			goto exit_gcm;
		}

		memcpy(pt, buf, dec_len);
		pt += dec_len;
		len -= dec_len;
	}

#if (MBEDTLS_VERSION_MAJOR < 3)
	rc = mbedtls_gcm_finish(&ctx, tag_buf, sizeof(tag_buf));
#else
	rc = mbedtls_gcm_finish(&ctx, NULL, 0, &output_length, tag_buf, sizeof(tag_buf));
#endif

	if (rc != 0) {
		rc = CRYPTO_ERR_DECRYPTION;
		goto exit_gcm;
	}

	/* Check tag in "constant-time" */
	for (diff = 0, i = 0; i < tag_len; i++)
		diff |= ((const unsigned char *)tag)[i] ^ tag_buf[i];

	if (diff != 0) {
		rc = CRYPTO_ERR_DECRYPTION;
		goto exit_gcm;
	}

	/* GCM decryption success */
	rc = CRYPTO_SUCCESS;

exit_gcm:
	mbedtls_gcm_free(&ctx);
	return rc;
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
