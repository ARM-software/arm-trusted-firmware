/*
 * Copyright (c) 2015-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>
#if OPENSSL_VERSION_NUMBER >= 0x30200000L
#include <openssl/core_names.h>
#include <openssl/params.h>
#endif

#include "cert.h"
#include "cmd_opt.h"
#include "debug.h"
#include "key.h"
#include "sha.h"

#define SERIAL_RAND_BITS	64

cert_t *certs;
unsigned int num_certs;

int rand_serial(BIGNUM *b, ASN1_INTEGER *ai)
{
	BIGNUM *btmp;
	int ret = 0;
	if (b)
		btmp = b;
	else
		btmp = BN_new();

	if (!btmp)
		return 0;

#if USING_OPENSSL3
	if (!BN_rand(btmp, SERIAL_RAND_BITS, 0, 0))
#else
	if (!BN_pseudo_rand(btmp, SERIAL_RAND_BITS, 0, 0))
#endif
		goto error;
	if (ai && !BN_to_ASN1_INTEGER(btmp, ai))
		goto error;

	ret = 1;

error:

	if (!b)
		BN_free(btmp);

	return ret;
}

/*
 * Read SOURCE_DATE_EPOCH (https://reproducible-builds.org/specs/source-date-epoch/)
 * into 't'. Returns 1 if it is set, 0 if it is not and -1 if it is malformed.
 */
static int source_date_epoch(time_t *t)
{
	const char *env = getenv("SOURCE_DATE_EPOCH");
	unsigned long long epoch;
	char *end;

	if (env == NULL || *env == '\0') {
		return 0;
	}

	errno = 0;
	epoch = strtoull(env, &end, 10);
	if (errno != 0 || *end != '\0' || *env < '0' || *env > '9' ||
	    epoch != (unsigned long long)(time_t)epoch) {
		ERROR("Invalid SOURCE_DATE_EPOCH '%s'\n", env);
		return -1;
	}

	*t = (time_t)epoch;
	return 1;
}

/*
 * Derive the serial number of certificate 'x' from the digest of its subject,
 * public key and extensions, which hold the hashes of the images, so that it
 * is reproducible, yet differs between certificates.
 */
static int digest_serial(X509 *x)
{
	X509_EXTENSIONS *exts = (X509_EXTENSIONS *)X509_get0_extensions(x);
	unsigned char *subject = NULL, *pubkey = NULL, *ext = NULL;
	unsigned char md[EVP_MAX_MD_SIZE];
	int subject_len, pubkey_len, ext_len, ret = 0;
	EVP_MD_CTX *ctx = NULL;
	ASN1_INTEGER *sno;
	BIGNUM *bn = NULL;

	subject_len = i2d_X509_NAME(X509_get_subject_name(x), &subject);
	pubkey_len = i2d_X509_PUBKEY(X509_get_X509_PUBKEY(x), &pubkey);
	ext_len = exts != NULL ? i2d_X509_EXTENSIONS(exts, &ext) : 0;
	ctx = EVP_MD_CTX_create();
	if (subject_len <= 0 || pubkey_len <= 0 || ext_len < 0 ||
	    ctx == NULL ||
	    !EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) ||
	    !EVP_DigestUpdate(ctx, subject, subject_len) ||
	    !EVP_DigestUpdate(ctx, pubkey, pubkey_len) ||
	    !EVP_DigestUpdate(ctx, ext, ext_len) ||
	    !EVP_DigestFinal_ex(ctx, md, NULL)) {
		goto error;
	}

	/* Keep the serial number positive */
	md[0] &= 0x7f;
	bn = BN_bin2bn(md, SERIAL_RAND_BITS / 8, NULL);
	if (bn == NULL) {
		goto error;
	}

	sno = BN_to_ASN1_INTEGER(bn, NULL);
	if (sno == NULL) {
		goto error;
	}
	ret = X509_set_serialNumber(x, sno);
	ASN1_INTEGER_free(sno);

error:
	EVP_MD_CTX_destroy(ctx);
	BN_free(bn);
	OPENSSL_free(subject);
	OPENSSL_free(pubkey);
	OPENSSL_free(ext);
	return ret;
}

const EVP_MD *get_digest(int alg)
{
	switch (alg) {
	case HASH_ALG_SHA256:
		return EVP_sha256();
	case HASH_ALG_SHA384:
		return EVP_sha384();
	case HASH_ALG_SHA512:
		return EVP_sha512();
	default:
		return NULL;
	}
}

int cert_add_ext(X509 *issuer, X509 *subject, int nid, char *value)
{
	X509_EXTENSION *ex;
	X509V3_CTX ctx;

	/* No configuration database */
	X509V3_set_ctx_nodb(&ctx);

	/* Set issuer and subject certificates in the context */
	X509V3_set_ctx(&ctx, issuer, subject, NULL, NULL, 0);
	ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
	if (!ex) {
		ERR_print_errors_fp(stdout);
		return 0;
	}

	X509_add_ext(subject, ex, -1);
	X509_EXTENSION_free(ex);

	return 1;
}

int cert_new(
	int md_alg,
	cert_t *cert,
	int days,
	int ca,
	STACK_OF(X509_EXTENSION) * sk)
{
	EVP_PKEY *pkey = keys[cert->key].key;
	cert_t *issuer_cert = &certs[cert->issuer];
	EVP_PKEY *ikey = keys[issuer_cert->key].key;
	X509 *issuer = issuer_cert->x;
	X509 *x;
	X509_EXTENSION *ex;
	X509_NAME *name;
	ASN1_INTEGER *sno;
	int i, num, rc = 0;
	EVP_MD_CTX *mdCtx;
	EVP_PKEY_CTX *pKeyCtx = NULL;
	time_t epoch, *t = NULL;

	/* Create the certificate structure */
	x = X509_new();
	if (!x) {
		return 0;
	}

	/* If we do not have a key, use the issuer key (the certificate will
	 * become self signed). This happens in content certificates. */
	if (!pkey) {
		pkey = ikey;
	}

	/* If we do not have an issuer certificate, use our own (the certificate
	 * will become self signed) */
	if (!issuer) {
		issuer = x;
	}

	mdCtx = EVP_MD_CTX_create();
	if (mdCtx == NULL) {
		ERR_print_errors_fp(stdout);
		goto END;
	}

	/* Sign the certificate with the issuer key */
	if (!EVP_DigestSignInit(mdCtx, &pKeyCtx, get_digest(md_alg), NULL, ikey)) {
		ERR_print_errors_fp(stdout);
		goto END;
	}

#if OPENSSL_VERSION_NUMBER >= 0x30200000L
	/*
	 * Derive the ECDSA nonce from the key and the message (RFC 6979)
	 * instead of drawing it at random, so that the same certificate always
	 * gets the same signature.
	 */
	if (EVP_PKEY_is_a(ikey, "EC")) {
		unsigned int nonce_type = 1;
		OSSL_PARAM params[] = {
			OSSL_PARAM_construct_uint(OSSL_SIGNATURE_PARAM_NONCE_TYPE,
						  &nonce_type),
			OSSL_PARAM_construct_end()
		};

		if (!EVP_PKEY_CTX_set_params(pKeyCtx, params)) {
			ERR_print_errors_fp(stdout);
			goto END;
		}
	}
#endif

	/*
	 * Set additional parameters if issuing public key algorithm is RSA.
	 * This is not required for ECDSA.
	 */
	if (EVP_PKEY_base_id(ikey) == EVP_PKEY_RSA) {
		if (!EVP_PKEY_CTX_set_rsa_padding(pKeyCtx, RSA_PKCS1_PSS_PADDING)) {
			ERR_print_errors_fp(stdout);
			goto END;
		}

		if (!EVP_PKEY_CTX_set_rsa_pss_saltlen(pKeyCtx, EVP_MD_size(get_digest(md_alg)))) {
			ERR_print_errors_fp(stdout);
			goto END;
		}

		if (!EVP_PKEY_CTX_set_rsa_mgf1_md(pKeyCtx, get_digest(md_alg))) {
			ERR_print_errors_fp(stdout);
			goto END;
		}
	}

	/*
	 * With SOURCE_DATE_EPOCH set, the validity starts at SOURCE_DATE_EPOCH
	 * instead of the current time, and the serial number is derived from
	 * the contents of the certificate instead of being random.
	 */
	switch (source_date_epoch(&epoch)) {
	case 1:
		t = &epoch;
		break;
	case -1:
		goto END;
	}

	/* x509.v3 */
	X509_set_version(x, 2);

	/* Random serial number */
	if (t == NULL) {
		sno = ASN1_INTEGER_new();
		rand_serial(NULL, sno);
		X509_set_serialNumber(x, sno);
		ASN1_INTEGER_free(sno);
	}

	X509_time_adj_ex(X509_get_notBefore(x), 0, 0, t);
	X509_time_adj_ex(X509_get_notAfter(x), days, 0, t);
	X509_set_pubkey(x, pkey);

	/* Subject name */
	name = X509_get_subject_name(x);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
			(const unsigned char *)cert->cn, -1, -1, 0);
	X509_set_subject_name(x, name);

	/* Issuer name */
	name = X509_get_issuer_name(x);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
			(const unsigned char *)issuer_cert->cn, -1, -1, 0);
	X509_set_issuer_name(x, name);

	/* Add various extensions: standard extensions */
	cert_add_ext(issuer, x, NID_subject_key_identifier, "hash");
	cert_add_ext(issuer, x, NID_authority_key_identifier, "keyid:always");
	if (ca) {
		cert_add_ext(issuer, x, NID_basic_constraints, "CA:TRUE");
		cert_add_ext(issuer, x, NID_key_usage, "keyCertSign");
	} else {
		cert_add_ext(issuer, x, NID_basic_constraints, "CA:FALSE");
	}

	/* Add custom extensions */
	if (sk != NULL) {
		num = sk_X509_EXTENSION_num(sk);
		for (i = 0; i < num; i++) {
			ex = sk_X509_EXTENSION_value(sk, i);
			X509_add_ext(x, ex, -1);
		}
	}

	if (t != NULL && !digest_serial(x)) {
		ERR_print_errors_fp(stdout);
		goto END;
	}

	if (!X509_sign_ctx(x, mdCtx)) {
		ERR_print_errors_fp(stdout);
		goto END;
	}

	/* X509 certificate signed successfully */
	rc = 1;
	cert->x = x;

END:
	EVP_MD_CTX_destroy(mdCtx);
	return rc;
}

int cert_init(void)
{
	cmd_opt_t cmd_opt;
	cert_t *cert;
	unsigned int i;

	certs = malloc((num_def_certs * sizeof(def_certs[0]))
#ifdef PDEF_CERTS
		       + (num_pdef_certs * sizeof(pdef_certs[0]))
#endif
		       );
	if (certs == NULL) {
		ERROR("%s:%d Failed to allocate memory.\n", __func__, __LINE__);
		return 1;
	}

	memcpy(&certs[0], &def_certs[0],
	       (num_def_certs * sizeof(def_certs[0])));

#ifdef PDEF_CERTS
	memcpy(&certs[num_def_certs], &pdef_certs[0],
	       (num_pdef_certs * sizeof(pdef_certs[0])));

	num_certs = num_def_certs + num_pdef_certs;
#else
	num_certs = num_def_certs;
#endif

	for (i = 0; i < num_certs; i++) {
		cert = &certs[i];
		cmd_opt.long_opt.name = cert->opt;
		cmd_opt.long_opt.has_arg = required_argument;
		cmd_opt.long_opt.flag = NULL;
		cmd_opt.long_opt.val = CMD_OPT_CERT;
		cmd_opt.help_msg = cert->help_msg;
		cmd_opt_add(&cmd_opt);
	}

	return 0;
}

cert_t *cert_get_by_opt(const char *opt)
{
	cert_t *cert;
	unsigned int i;

	for (i = 0; i < num_certs; i++) {
		cert = &certs[i];
		if (0 == strcmp(cert->opt, opt)) {
			return cert;
		}
	}

	return NULL;
}

void cert_cleanup(void)
{
	unsigned int i;

	for (i = 0; i < num_certs; i++) {
		if (certs[i].fn != NULL) {
			void *ptr = (void *)certs[i].fn;

			certs[i].fn = NULL;
			free(ptr);
		}
	}
	free(certs);
}

