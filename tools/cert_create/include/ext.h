/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EXT_H
#define EXT_H

#include <openssl/x509v3.h>
#include "key.h"

/* Extension types supported */
enum ext_type_e {
	EXT_TYPE_NVCOUNTER,
	EXT_TYPE_PKEY,
	EXT_TYPE_HASH
};

/* NV-Counter types */
enum nvctr_type_e {
	NVCTR_TYPE_TFW,
	NVCTR_TYPE_NTFW
};

/*
 * This structure contains the relevant information to create the extensions
 * to be included in the certificates. This extensions will be used to
 * establish the chain of trust.
 */
typedef struct ext_s {
	const char *oid;	/* OID of the extension */
	const char *sn;		/* Short name */
	const char *ln;		/* Long description */
	const char *opt;	/* Command line option to specify data */
	const char *help_msg;	/* Help message */
	const char *arg;	/* Argument passed from command line */
	int asn1_type;		/* OpenSSL ASN1 type of the extension data.
				 * Supported types are:
				 *   - V_ASN1_INTEGER
				 *   - V_ASN1_OCTET_STRING
				 */
	int type;		/* See ext_type_e */

	/* Extension attributes (depends on extension type) */
	union {
		int nvctr_type;	/* See nvctr_type_e */
		int key;	/* Index into array of registered public keys */
	} attr;

	int alias;		/* In case OpenSSL provides an standard
				 * extension of the same type, add the new
				 * extension as an alias of this one
				 */

	X509V3_EXT_METHOD method; /* This field may be used to define a custom
				   * function to print the contents of the
				   * extension */

	int optional;	/* This field may be used optionally to exclude an image */
} ext_t;

enum {
	EXT_NON_CRIT = 0,
	EXT_CRIT = !EXT_NON_CRIT,
};

/* Exported API */
int ext_init(void);
ext_t *ext_get_by_opt(const char *opt);
X509_EXTENSION *ext_new_hash(int nid, int crit, const EVP_MD *md,
		unsigned char *buf, size_t len);
X509_EXTENSION *ext_new_nvcounter(int nid, int crit, int value);
X509_EXTENSION *ext_new_key(int nid, int crit, EVP_PKEY *k);

/* Macro to register the extensions used in the CoT */
#define REGISTER_EXTENSIONS(_ext) \
	ext_t *def_extensions = &_ext[0]; \
	const unsigned int num_def_extensions = sizeof(_ext)/sizeof(_ext[0])

/* Macro to register the platform defined extensions used in the CoT */
#define PLAT_REGISTER_EXTENSIONS(_pdef_ext) \
	ext_t *pdef_extensions = &_pdef_ext[0]; \
	const unsigned int num_pdef_extensions = sizeof(_pdef_ext)/sizeof(_pdef_ext[0])

/* Exported variables */
extern ext_t *def_extensions;
extern const unsigned int num_def_extensions;
extern ext_t *pdef_extensions;
extern const unsigned int num_pdef_extensions;

extern ext_t *extensions;
extern unsigned int num_extensions;
#endif /* EXT_H */
