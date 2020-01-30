/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#include "cmd_opt.h"
#include "ext.h"

DECLARE_ASN1_ITEM(ASN1_INTEGER)
DECLARE_ASN1_ITEM(X509_ALGOR)
DECLARE_ASN1_ITEM(ASN1_OCTET_STRING)

typedef struct {
	X509_ALGOR *hashAlgorithm;
	ASN1_OCTET_STRING *dataHash;
} HASH;

ASN1_SEQUENCE(HASH) = {
	ASN1_SIMPLE(HASH, hashAlgorithm, X509_ALGOR),
	ASN1_SIMPLE(HASH, dataHash, ASN1_OCTET_STRING),
} ASN1_SEQUENCE_END(HASH)

DECLARE_ASN1_FUNCTIONS(HASH)
IMPLEMENT_ASN1_FUNCTIONS(HASH)

/*
 * This function adds the CoT extensions to the internal extension list
 * maintained by OpenSSL so they can be used later.
 *
 * It also initializes the methods to print the contents of the extension. If an
 * alias is specified in the CoT extension, we reuse the methods of the alias.
 * Otherwise, only methods for V_ASN1_INTEGER and V_ASN1_OCTET_STRING are
 * provided. Any other type will be printed as a raw ascii string.
 *
 * Return: 0 = success, Otherwise: error
 */
int ext_init(void)
{
	cmd_opt_t cmd_opt;
	ext_t *ext;
	X509V3_EXT_METHOD *m;
	int nid, ret;
	unsigned int i;

	for (i = 0; i < num_extensions; i++) {
		ext = &extensions[i];
		/* Register command line option */
		if (ext->opt) {
			cmd_opt.long_opt.name = ext->opt;
			cmd_opt.long_opt.has_arg = required_argument;
			cmd_opt.long_opt.flag = NULL;
			cmd_opt.long_opt.val = CMD_OPT_EXT;
			cmd_opt.help_msg = ext->help_msg;
			cmd_opt_add(&cmd_opt);
		}
		/* Register the extension OID in OpenSSL */
		if (ext->oid == NULL) {
			continue;
		}
		nid = OBJ_create(ext->oid, ext->sn, ext->ln);
		if (ext->alias) {
			X509V3_EXT_add_alias(nid, ext->alias);
		} else {
			m = &ext->method;
			memset(m, 0x0, sizeof(X509V3_EXT_METHOD));
			switch (ext->asn1_type) {
			case V_ASN1_INTEGER:
				m->it = ASN1_ITEM_ref(ASN1_INTEGER);
				m->i2s = (X509V3_EXT_I2S)i2s_ASN1_INTEGER;
				m->s2i = (X509V3_EXT_S2I)s2i_ASN1_INTEGER;
				break;
			case V_ASN1_OCTET_STRING:
				m->it = ASN1_ITEM_ref(ASN1_OCTET_STRING);
				m->i2s = (X509V3_EXT_I2S)i2s_ASN1_OCTET_STRING;
				m->s2i = (X509V3_EXT_S2I)s2i_ASN1_OCTET_STRING;
				break;
			default:
				continue;
			}
			m->ext_nid = nid;
			ret = X509V3_EXT_add(m);
			if (!ret) {
				ERR_print_errors_fp(stdout);
				return 1;
			}
		}
	}
	return 0;
}

/*
 * Create a new extension
 *
 * Extension  ::=  SEQUENCE  {
 *      id          OBJECT IDENTIFIER,
 *      critical    BOOLEAN DEFAULT FALSE,
 *      value       OCTET STRING  }
 *
 * Parameters:
 *   pex: OpenSSL extension pointer (output parameter)
 *   nid: extension identifier
 *   crit: extension critical (EXT_NON_CRIT, EXT_CRIT)
 *   data: extension data. This data will be encapsulated in an Octet String
 *
 * Return: Extension address, NULL if error
 */
static
X509_EXTENSION *ext_new(int nid, int crit, unsigned char *data, int len)
{
	X509_EXTENSION *ex;
	ASN1_OCTET_STRING *ext_data;

	/* Octet string containing the extension data */
	ext_data = ASN1_OCTET_STRING_new();
	ASN1_OCTET_STRING_set(ext_data, data, len);

	/* Create the extension */
	ex = X509_EXTENSION_create_by_NID(NULL, nid, crit, ext_data);

	/* The extension makes a copy of the data, so we can free this object */
	ASN1_OCTET_STRING_free(ext_data);

	return ex;
}

/*
 * Creates a x509v3 extension containing a hash
 *
 * DigestInfo ::= SEQUENCE {
 *     digestAlgorithm  AlgorithmIdentifier,
 *     digest           OCTET STRING
 * }
 *
 * AlgorithmIdentifier ::=  SEQUENCE  {
 *     algorithm        OBJECT IDENTIFIER,
 *     parameters       ANY DEFINED BY algorithm OPTIONAL
 * }
 *
 * Parameters:
 *   nid: extension identifier
 *   crit: extension critical (EXT_NON_CRIT, EXT_CRIT)
 *   md: hash algorithm
 *   buf: pointer to the buffer that contains the hash
 *   len: size of the hash in bytes
 *
 * Return: Extension address, NULL if error
 */
X509_EXTENSION *ext_new_hash(int nid, int crit, const EVP_MD *md,
		unsigned char *buf, size_t len)
{
	X509_EXTENSION *ex;
	ASN1_OCTET_STRING *octet;
	HASH *hash;
	ASN1_OBJECT *algorithm;
	X509_ALGOR *x509_algor;
	unsigned char *p = NULL;
	int sz;

	/* OBJECT_IDENTIFIER with hash algorithm */
	algorithm = OBJ_nid2obj(EVP_MD_type(md));
	if (algorithm == NULL) {
		return NULL;
	}

	/* Create X509_ALGOR */
	x509_algor = X509_ALGOR_new();
	if (x509_algor == NULL) {
		return NULL;
	}
	x509_algor->algorithm = algorithm;
	x509_algor->parameter = ASN1_TYPE_new();
	ASN1_TYPE_set(x509_algor->parameter, V_ASN1_NULL, NULL);

	/* OCTET_STRING with the actual hash */
	octet = ASN1_OCTET_STRING_new();
	if (octet == NULL) {
		X509_ALGOR_free(x509_algor);
		return NULL;
	}
	ASN1_OCTET_STRING_set(octet, buf, len);

	/* HASH structure containing algorithm + hash */
	hash = HASH_new();
	if (hash == NULL) {
		ASN1_OCTET_STRING_free(octet);
		X509_ALGOR_free(x509_algor);
		return NULL;
	}
	hash->hashAlgorithm = x509_algor;
	hash->dataHash = octet;

	/* DER encoded HASH */
	sz = i2d_HASH(hash, &p);
	if ((sz <= 0) || (p == NULL)) {
		HASH_free(hash);
		X509_ALGOR_free(x509_algor);
		return NULL;
	}

	/* Create the extension */
	ex = ext_new(nid, crit, p, sz);

	/* Clean up */
	OPENSSL_free(p);
	HASH_free(hash);

	return ex;
}

/*
 * Creates a x509v3 extension containing a nvcounter encapsulated in an ASN1
 * Integer
 *
 * Parameters:
 *   pex: OpenSSL extension pointer (output parameter)
 *   nid: extension identifier
 *   crit: extension critical (EXT_NON_CRIT, EXT_CRIT)
 *   value: nvcounter value
 *
 * Return: Extension address, NULL if error
 */
X509_EXTENSION *ext_new_nvcounter(int nid, int crit, int value)
{
	X509_EXTENSION *ex;
	ASN1_INTEGER *counter;
	unsigned char *p = NULL;
	int sz;

	/* Encode counter */
	counter = ASN1_INTEGER_new();
	ASN1_INTEGER_set(counter, value);
	sz = i2d_ASN1_INTEGER(counter, &p);

	/* Create the extension */
	ex = ext_new(nid, crit, p, sz);

	/* Free objects */
	OPENSSL_free(p);
	ASN1_INTEGER_free(counter);

	return ex;
}

/*
 * Creates a x509v3 extension containing a public key in DER format:
 *
 *  SubjectPublicKeyInfo  ::=  SEQUENCE  {
 *       algorithm            AlgorithmIdentifier,
 *       subjectPublicKey     BIT STRING }
 *
 * Parameters:
 *   pex: OpenSSL extension pointer (output parameter)
 *   nid: extension identifier
 *   crit: extension critical (EXT_NON_CRIT, EXT_CRIT)
 *   k: key
 *
 * Return: Extension address, NULL if error
 */
X509_EXTENSION *ext_new_key(int nid, int crit, EVP_PKEY *k)
{
	X509_EXTENSION *ex;
	unsigned char *p;
	int sz;

	/* Encode key */
	BIO *mem = BIO_new(BIO_s_mem());
	if (i2d_PUBKEY_bio(mem, k) <= 0) {
		ERR_print_errors_fp(stderr);
		return NULL;
	}
	p = (unsigned char *)OPENSSL_malloc(4096);
	sz = BIO_read(mem, p, 4096);

	/* Create the extension */
	ex = ext_new(nid, crit, p, sz);

	/* Clean up */
	BIO_free(mem);
	OPENSSL_free(p);

	return ex;
}

ext_t *ext_get_by_opt(const char *opt)
{
	ext_t *ext;
	unsigned int i;

	/* Sequential search. This is not a performance concern since the number
	 * of extensions is bounded and the code runs on a host machine */
	for (i = 0; i < num_extensions; i++) {
		ext = &extensions[i];
		if (ext->opt && !strcmp(ext->opt, opt)) {
			return ext;
		}
	}

	return NULL;
}
