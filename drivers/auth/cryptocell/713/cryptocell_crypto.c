/*
 * Copyright (c) 2017-2020 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <platform_def.h>

#include <drivers/arm/cryptocell/713/bsv_api.h>
#include <drivers/arm/cryptocell/713/bsv_crypto_asym_api.h>
#include <drivers/auth/crypto_mod.h>

#include <mbedtls/oid.h>
#include <mbedtls/x509.h>

#define LIB_NAME		"CryptoCell 713 SBROM"
#define RSA_SALT_LEN		32
#define RSA_EXPONENT		65537

/*
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm            OBJECT IDENTIFIER,
 *     parameters           ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * SubjectPublicKeyInfo  ::=  SEQUENCE  {
 *     algorithm            AlgorithmIdentifier,
 *     subjectPublicKey     BIT STRING
 * }
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm      AlgorithmIdentifier,
 *     digest               OCTET STRING
 * }
 *
 *  RSASSA-PSS-params ::= SEQUENCE {
 *     hashAlgorithm        [0] HashAlgorithm,
 *     maskGenAlgorithm     [1] MaskGenAlgorithm,
 *     saltLength           [2] INTEGER,
 *     trailerField         [3] TrailerField    DEFAULT trailerFieldBC
 * }
 */

/*
 * Initialize the library and export the descriptor
 */
static void init(void)
{
	CCError_t ret;
	uint32_t lcs;

	/* Initialize CC SBROM */
	ret = CC_BsvInit((uintptr_t)PLAT_CRYPTOCELL_BASE);
	if (ret != CC_OK) {
		ERROR("CryptoCell CC_BsvInit() error %x\n", ret);
		panic();
	}

	/* Initialize lifecycle state */
	ret = CC_BsvGetAndInitLcs((uintptr_t)PLAT_CRYPTOCELL_BASE, &lcs);
	if (ret != CC_OK) {
		ERROR("CryptoCell CC_BsvGetAndInitLcs() error %x\n", ret);
		panic();
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
	CCError_t error;
	CCBsvNBuff_t NBuff;
	CCBsvSignature_t signature;
	int rc, exp;
	mbedtls_asn1_buf sig_oid, alg_oid, params;
	mbedtls_md_type_t md_alg;
	mbedtls_pk_type_t pk_alg;
	mbedtls_pk_rsassa_pss_options pss_opts;
	size_t len;
	uint8_t *p, *end;
	CCHashResult_t digest;
	CCBool_t is_verified;
	/* This is a rather large array, we don't want it on stack */
	static uint32_t workspace[BSV_RSA_WORKSPACE_MIN_SIZE];

	/* Verify the signature algorithm */
	/* Get pointers to signature OID and parameters */
	p = sig_alg;
	end = p + sig_alg_len;
	rc = mbedtls_asn1_get_alg(&p, end, &sig_oid, &params);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	/* Get the actual signature algorithm (MD + PK) */
	rc = mbedtls_oid_get_sig_alg(&sig_oid, &md_alg, &pk_alg);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	/* The CryptoCell only supports RSASSA-PSS signature */
	if (pk_alg != MBEDTLS_PK_RSASSA_PSS || md_alg != MBEDTLS_MD_NONE)
		return CRYPTO_ERR_SIGNATURE;

	/* Verify the RSASSA-PSS params */
	/* The trailer field is verified to be 0xBC internally by this API */
	rc = mbedtls_x509_get_rsassa_pss_params(&params, &md_alg,
			&pss_opts.mgf1_hash_id,
			&pss_opts.expected_salt_len);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	/* The CryptoCell only supports SHA256 as hash algorithm */
	if (md_alg != MBEDTLS_MD_SHA256 ||
	    pss_opts.mgf1_hash_id != MBEDTLS_MD_SHA256)
		return CRYPTO_ERR_SIGNATURE;

	if (pss_opts.expected_salt_len != RSA_SALT_LEN)
		return CRYPTO_ERR_SIGNATURE;

	/* Parse the public key */
	p = pk_ptr;
	end = p + pk_len;
	rc = mbedtls_asn1_get_tag(&p, end, &len,
			MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	end = p + len;
	rc = mbedtls_asn1_get_alg_null(&p, end, &alg_oid);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	if (mbedtls_oid_get_pk_alg(&alg_oid, &pk_alg) != 0)
		return CRYPTO_ERR_SIGNATURE;

	if (pk_alg != MBEDTLS_PK_RSA)
		return CRYPTO_ERR_SIGNATURE;

	rc = mbedtls_asn1_get_bitstring_null(&p, end, &len);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	rc = mbedtls_asn1_get_tag(&p, end, &len,
				MBEDTLS_ASN1_CONSTRUCTED |
				MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_INTEGER);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	if (*p == 0) {
		p++; len--;
	}
	if (len != BSV_CERT_RSA_KEY_SIZE_IN_BYTES || ((p + len) > end))
		return CRYPTO_ERR_SIGNATURE;

	/*
	 * Copy N from certificate.
	 */
	memcpy(NBuff, p, BSV_CERT_RSA_KEY_SIZE_IN_BYTES);

	/* Verify the RSA exponent */
	p += len;
	rc = mbedtls_asn1_get_int(&p, end, &exp);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	if (exp != RSA_EXPONENT)
		return CRYPTO_ERR_SIGNATURE;

	/* Get the signature (bitstring) */
	p = sig_ptr;
	end = p + sig_len;
	rc = mbedtls_asn1_get_bitstring_null(&p, end, &len);
	if (rc != 0)
		return CRYPTO_ERR_SIGNATURE;

	if (len != BSV_CERT_RSA_KEY_SIZE_IN_BYTES || ((p + len) > end))
		return CRYPTO_ERR_SIGNATURE;

	/*
	 * Copy the signature (in BE format)
	 */
	memcpy((uint8_t *)signature, p, BSV_CERT_RSA_KEY_SIZE_IN_BYTES);

	error = CC_BsvSha256((uintptr_t)PLAT_CRYPTOCELL_BASE,
			     data_ptr, data_len, digest);
	if (error != CC_OK)
		return CRYPTO_ERR_SIGNATURE;

	/* Verify the signature */
	error = CC_BsvRsaPssVerify((uintptr_t)PLAT_CRYPTOCELL_BASE, NBuff,
				    NULL, signature, digest, workspace,
				    BSV_RSA_WORKSPACE_MIN_SIZE, &is_verified);
	if ((error != CC_OK) || (is_verified != CC_TRUE))
		return CRYPTO_ERR_SIGNATURE;

	/* Signature verification success */
	return CRYPTO_SUCCESS;
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
	uint8_t *p, *end, *hash;
	CCHashResult_t pubKeyHash;
	size_t len;
	int rc;
	CCError_t error;

	/* Digest info should be an MBEDTLS_ASN1_SEQUENCE */
	p = digest_info_ptr;
	end = p + digest_info_len;
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED |
				  MBEDTLS_ASN1_SEQUENCE);
	if (rc != 0)
		return CRYPTO_ERR_HASH;

	/* Get the hash algorithm */
	rc = mbedtls_asn1_get_alg(&p, end, &hash_oid, &params);
	if (rc != 0)
		return CRYPTO_ERR_HASH;

	rc = mbedtls_oid_get_md_alg(&hash_oid, &md_alg);
	if (rc != 0)
		return CRYPTO_ERR_HASH;
	/* Verify that hash algorithm is SHA256 */
	if (md_alg != MBEDTLS_MD_SHA256)
		return CRYPTO_ERR_HASH;

	/* Hash should be octet string type */
	rc = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
	if (rc != 0)
		return CRYPTO_ERR_HASH;

	/* Length of hash must match the algorithm's size */
	if (len != HASH_RESULT_SIZE_IN_BYTES)
		return CRYPTO_ERR_HASH;

	hash = p;
	error = CC_BsvSha256((uintptr_t)PLAT_CRYPTOCELL_BASE, data_ptr,
			     data_len, pubKeyHash);
	if (error != CC_OK)
		return CRYPTO_ERR_HASH;

	rc = memcmp(pubKeyHash, hash, HASH_RESULT_SIZE_IN_BYTES);
	if (rc != 0)
		return CRYPTO_ERR_HASH;

	return CRYPTO_SUCCESS;
}

/*
 * Register crypto library descriptor
 */
REGISTER_CRYPTO_LIB(LIB_NAME, init, verify_signature, verify_hash, NULL);
