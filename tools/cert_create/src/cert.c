/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>

#if USE_TBBR_DEFS
#include <tbbr_oid.h>
#else
#include <platform_oid.h>
#endif

#include "cert.h"
#include "cmd_opt.h"
#include "debug.h"
#include "key.h"
#include "sha.h"

#define SERIAL_RAND_BITS	64
#define RSA_SALT_LEN		32

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

	if (!BN_pseudo_rand(btmp, SERIAL_RAND_BITS, 0, 0))
		goto error;
	if (ai && !BN_to_ASN1_INTEGER(btmp, ai))
		goto error;

	ret = 1;

error:

	if (!b)
		BN_free(btmp);

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

	/*
	 * Set additional parameters if issuing public key algorithm is RSA.
	 * This is not required for ECDSA.
	 */
	if (EVP_PKEY_base_id(ikey) == EVP_PKEY_RSA) {
		if (!EVP_PKEY_CTX_set_rsa_padding(pKeyCtx, RSA_PKCS1_PSS_PADDING)) {
			ERR_print_errors_fp(stdout);
			goto END;
		}

		if (!EVP_PKEY_CTX_set_rsa_pss_saltlen(pKeyCtx, RSA_SALT_LEN)) {
			ERR_print_errors_fp(stdout);
			goto END;
		}

		if (!EVP_PKEY_CTX_set_rsa_mgf1_md(pKeyCtx, get_digest(md_alg))) {
			ERR_print_errors_fp(stdout);
			goto END;
		}
	}

	/* x509.v3 */
	X509_set_version(x, 2);

	/* Random serial number */
	sno = ASN1_INTEGER_new();
	rand_serial(NULL, sno);
	X509_set_serialNumber(x, sno);
	ASN1_INTEGER_free(sno);

	X509_gmtime_adj(X509_get_notBefore(x), 0);
	X509_gmtime_adj(X509_get_notAfter(x), (long)60*60*24*days);
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
