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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>

#include "cert.h"
#include "cmd_opt.h"
#include "debug.h"
#include "key.h"
#include "platform_oid.h"
#include "sha.h"

#define SERIAL_RAND_BITS	64

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


int cert_new(cert_t *cert, int days, int ca, STACK_OF(X509_EXTENSION) * sk)
{
	EVP_PKEY *pkey = keys[cert->key].key;
	cert_t *issuer_cert = &certs[cert->issuer];
	EVP_PKEY *ikey = keys[issuer_cert->key].key;
	X509 *issuer = issuer_cert->x;
	X509 *x = NULL;
	X509_EXTENSION *ex = NULL;
	X509_NAME *name = NULL;
	ASN1_INTEGER *sno = NULL;
	int i, num;

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

	/* Sign the certificate with the issuer key */
	if (!X509_sign(x, ikey, EVP_sha256())) {
		ERR_print_errors_fp(stdout);
		return 0;
	}

	cert->x = x;
	return 1;
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
	cert_t *cert = NULL;
	unsigned int i;

	for (i = 0; i < num_certs; i++) {
		cert = &certs[i];
		if (0 == strcmp(cert->opt, opt)) {
			return cert;
		}
	}

	return NULL;
}
