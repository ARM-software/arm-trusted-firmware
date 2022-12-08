/*
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
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
#include <mbedtls/x509.h>

#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/auth/mbedtls/mbedtls_common.h>
#include <drivers/auth/mbedtls/mbedtls_config.h>
#include <plat/common/platform.h>

#define LIB_NAME		"mbed TLS"

#if CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
/*
 * CRYPTO_MD_MAX_SIZE value is as per current stronger algorithm available
 * so make sure that mbed TLS MD maximum size must be lesser than this.
 */
CASSERT(CRYPTO_MD_MAX_SIZE >= MBEDTLS_MD_MAX_SIZE,
	assert_mbedtls_md_size_overflow);

#endif /* CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

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

#if CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC
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
	if ((rc != 0) || ((size_t)(end - p) != signature.len)) {
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

	/*
	 * Digest info should be an MBEDTLS_ASN1_SEQUENCE
	 * and consume all bytes.
	 */
	p = (unsigned char *)digest_info_ptr;
	end = p + digest_info_len;
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				  MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0 || ((size_t)(end - p) != len)) {
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

	/* Hash should be octet string type and consume all bytes */
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
	if ((rc != 0) || ((size_t)(end - p) != len)) {
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
#endif /* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

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
	const mbedtls_md_info_t *md_info;

	md_info = mbedtls_md_info_from_type(md_type(md_algo));
	if (md_info == NULL) {
		return CRYPTO_ERR_HASH;
	}

	/*
	 * Calculate the hash of the data, it is safe to pass the
	 * 'output' hash buffer pointer considering its size is always
	 * bigger than or equal to MBEDTLS_MD_MAX_SIZE.
	 */
	return mbedtls_md(md_info, data_ptr, data_len, output);
}
#endif /* CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY || \
	  CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */

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

	mbedtls_gcm_init(&ctx);

	rc = mbedtls_gcm_setkey(&ctx, cipher, key, key_len * 8);
	if (rc != 0) {
		rc = CRYPTO_ERR_DECRYPTION;
		goto exit_gcm;
	}

	rc = mbedtls_gcm_starts(&ctx, MBEDTLS_GCM_DECRYPT, iv, iv_len, NULL, 0);
	if (rc != 0) {
		rc = CRYPTO_ERR_DECRYPTION;
		goto exit_gcm;
	}

	while (len > 0) {
		dec_len = MIN(sizeof(buf), len);

		rc = mbedtls_gcm_update(&ctx, dec_len, pt, buf);
		if (rc != 0) {
			rc = CRYPTO_ERR_DECRYPTION;
			goto exit_gcm;
		}

		memcpy(pt, buf, dec_len);
		pt += dec_len;
		len -= dec_len;
	}

	rc = mbedtls_gcm_finish(&ctx, tag_buf, sizeof(tag_buf));
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
		    auth_decrypt);
#else
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, calc_hash,
		    NULL);
#endif
#elif CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_ONLY
#if TF_MBEDTLS_USE_AES_GCM
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash,
		    auth_decrypt);
#else
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, NULL);
#endif
#elif CRYPTO_SUPPORT == CRYPTO_HASH_CALC_ONLY
REGISTER_CRYPTO_LIB(LIB_NAME, init, calc_hash);
#endif /* CRYPTO_SUPPORT == CRYPTO_AUTH_VERIFY_AND_HASH_CALC */
