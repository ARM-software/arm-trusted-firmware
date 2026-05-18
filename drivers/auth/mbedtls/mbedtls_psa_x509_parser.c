/*
 * Copyright (c) 2023-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

/* mbed TLS headers */
#include <mbedtls/asn1.h>
#include <mbedtls/oid.h>
#include <mbedtls/platform.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_crt.h>

#include <drivers/auth/img_parser_mod.h>
#include <drivers/auth/mbedtls/mbedtls_common.h>
#include "mbedtls_psa_internal.h"

/* Maximum OID string length ("a.b.c.d.e.f ...") */
#define MAX_OID_STR_LEN			64
#define LIB_NAME			"mbed TLS X509v3"
/* Reject the high-tag-number form in DER. */
#define ASN1_TAG_NUMBER_MASK			0x1FU
/* Reject the UNIVERSAL 0 tag (constructed or primitive). */
#define ASN1_TAG_WITHOUT_CONSTRUCTED_MASK	0xDFU

/* Temporary variables to speed up the authentication parameters search. */
static mbedtls_x509_buf cached_tbs;
static mbedtls_x509_buf cached_pk_raw;
static mbedtls_x509_buf cached_sig_alg;
static mbedtls_x509_buf cached_signature;
static extension_cache_t extension_cache[MBEDTLS_PSA_MAX_CACHED_EXTENSIONS];

static void clear_temp_vars(void)
{
	mbedtls_zero_and_clean(&cached_tbs, sizeof(cached_tbs));
	mbedtls_zero_and_clean(&cached_pk_raw, sizeof(cached_pk_raw));
	mbedtls_zero_and_clean(&cached_sig_alg, sizeof(cached_sig_alg));
	mbedtls_zero_and_clean(&cached_signature, sizeof(cached_signature));
}

static void invalidate_extension_cache(void)
{
	for (int i = 0; i < MBEDTLS_PSA_MAX_CACHED_EXTENSIONS; i++) {
		extension_cache[i].valid = false;
	}
}

static int get_cached_extension_info(const char *oid, void **ext,
				     unsigned int *ext_len)
{
	size_t extension_oid_len = strlen(oid);

	for (int i = 0; i < MBEDTLS_PSA_MAX_CACHED_EXTENSIONS; i++) {
		char oid_buf[MAX_OID_STR_LEN];
		int oid_len;

		if (!extension_cache[i].valid) {
			continue;
		}

		oid_len = mbedtls_oid_get_numeric_string(oid_buf, sizeof(oid_buf),
							 &extension_cache[i].extension_oid);
		if ((oid_len < 0) || ((size_t) oid_len != extension_oid_len)) {
			continue;
		}

		if (strncmp(oid_buf, oid, extension_oid_len) == 0) {
			*ext = extension_cache[i].ext;
			*ext_len = extension_cache[i].ext_len;
			return IMG_PARSER_OK;
		}
	}

	return IMG_PARSER_ERR_NOT_FOUND;
}

static int cache_extension_info(const mbedtls_x509_buf *extension_oid, void *ext,
				unsigned int ext_len)
{
	for (int i = 0; i < MBEDTLS_PSA_MAX_CACHED_EXTENSIONS; i++) {
		if (!extension_cache[i].valid) {
			extension_cache[i].extension_oid = *extension_oid;
			extension_cache[i].ext = ext;
			extension_cache[i].ext_len = ext_len;
			extension_cache[i].valid = true;
			return IMG_PARSER_OK;
		}
	}

	return MBEDTLS_ERR_X509_ALLOC_FAILED;
}

static int validate_extension_der_encoding(const unsigned char *p,
					   const unsigned char *end)
{
	unsigned char *der_p = (unsigned char *)p;
	size_t len = (size_t)(end - p);

	if (len < 2U) {
		return MBEDTLS_ERR_X509_INVALID_EXTENSIONS;
	}

	if ((der_p[0] & ASN1_TAG_NUMBER_MASK) == ASN1_TAG_NUMBER_MASK) {
		return MBEDTLS_ERR_X509_INVALID_EXTENSIONS;
	}

	if ((der_p[0] & ASN1_TAG_WITHOUT_CONSTRUCTED_MASK) == 0U) {
		return MBEDTLS_ERR_X509_INVALID_EXTENSIONS;
	}

	der_p++;

	if (mbedtls_asn1_get_len(&der_p, end, &len) != 0) {
		return MBEDTLS_ERR_X509_INVALID_EXTENSIONS;
	}

	if (der_p + len != end) {
		return MBEDTLS_ERR_X509_INVALID_EXTENSIONS;
	}

	return 0;
}

static int cache_signature_fields(const mbedtls_x509_crt *crt)
{
	unsigned char *p;
	const unsigned char *end;
	size_t len;
	int ret;

	p = crt->tbs.p + crt->tbs.len;
	end = crt->raw.p + crt->raw.len;

	cached_sig_alg.p = p;
	ret = mbedtls_asn1_get_tag(&p, end, &len,
				   MBEDTLS_ASN1_CONSTRUCTED |
				   MBEDTLS_ASN1_SEQUENCE);
	if (ret != 0) {
		return IMG_PARSER_ERR_FORMAT;
	}
	cached_sig_alg.tag = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;
	cached_sig_alg.len = (size_t)((p + len) - cached_sig_alg.p);
	p += len;

	cached_signature.p = p;
	ret = mbedtls_asn1_get_bitstring_null(&p, end, &len);
	if ((ret != 0) || ((p + len) != end)) {
		return IMG_PARSER_ERR_FORMAT;
	}
	cached_signature.tag = *cached_signature.p;
	cached_signature.len = (size_t)(end - cached_signature.p);

	return IMG_PARSER_OK;
}

static int extension_verifier(void *p_ctx, mbedtls_x509_crt const *crt,
			      mbedtls_x509_buf const *oid, int critical,
			      const unsigned char *p, const unsigned char *end)
{
	int ret;

	(void)p_ctx;
	(void)crt;
	(void)critical;

	ret = validate_extension_der_encoding(p, end);
	if (ret != 0) {
		return ret;
	}

	return cache_extension_info(oid, (void *)p, (unsigned int)(end - p));
}

static void init(void)
{
	mbedtls_init();
}

static int check_integrity(void *img, unsigned int img_len)
{
	mbedtls_x509_crt crt;
	int rc;

	invalidate_extension_cache();
	mbedtls_x509_crt_init(&crt);

	rc = mbedtls_x509_crt_parse_der_with_ext_cb(&crt, img, img_len, 0,
						    extension_verifier, NULL);
	if (rc != 0) {
		clear_temp_vars();
		mbedtls_x509_crt_free(&crt);
		return rc;
	}

	cached_tbs = crt.tbs;
	cached_pk_raw = crt.pk_raw;

	rc = cache_signature_fields(&crt);
	if (rc != 0) {
		clear_temp_vars();
		invalidate_extension_cache();
	}

	mbedtls_x509_crt_free(&crt);
	return rc;
}

static int get_auth_param(const auth_param_type_desc_t *type_desc,
			  void *img, unsigned int img_len,
			  void **param, unsigned int *param_len)
{
	int rc = IMG_PARSER_OK;

	(void)img;
	(void)img_len;

	switch (type_desc->type) {
	case AUTH_PARAM_RAW_DATA:
		*param = (void *)cached_tbs.p;
		*param_len = (unsigned int)cached_tbs.len;
		break;
	case AUTH_PARAM_HASH:
	case AUTH_PARAM_NV_CTR:
		rc = get_cached_extension_info(type_desc->cookie, param, param_len);
		break;
	case AUTH_PARAM_PUB_KEY:
		if (type_desc->cookie != NULL) {
			rc = get_cached_extension_info(type_desc->cookie, param, param_len);
		} else {
			*param = (void *)cached_pk_raw.p;
			*param_len = (unsigned int)cached_pk_raw.len;
		}
		break;
	case AUTH_PARAM_SIG_ALG:
		*param = (void *)cached_sig_alg.p;
		*param_len = (unsigned int)cached_sig_alg.len;
		break;
	case AUTH_PARAM_SIG:
		*param = (void *)cached_signature.p;
		*param_len = (unsigned int)cached_signature.len;
		break;
	default:
		rc = IMG_PARSER_ERR_NOT_FOUND;
		break;
	}

	return rc;
}

REGISTER_IMG_PARSER_LIB(IMG_CERT, LIB_NAME, init,
			check_integrity, get_auth_param);
