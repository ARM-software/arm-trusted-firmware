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

#ifndef EXT_H_
#define EXT_H_

#include "key.h"
#include <openssl/x509v3.h>

/* Extension types supported */
enum {
	EXT_TYPE_NVCOUNTER,
	EXT_TYPE_PKEY,
	EXT_TYPE_HASH
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
	int asn1_type;		/* OpenSSL ASN1 type of the extension data.
				 * Supported types are:
				 *   - V_ASN1_INTEGER
				 *   - V_ASN1_OCTET_STRING
				 */
	int type;
	const char *opt;	/* Command line option to specify data */
	/* Extension data (depends on extension type) */
	union {
		const char *fn;	/* File with extension data */
		int nvcounter;	/* Non volatile counter */
		int key;	/* Public key */
	} data;

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
	ext_t *extensions = &_ext[0]; \
	const unsigned int num_extensions = sizeof(_ext)/sizeof(_ext[0]);

/* Exported variables */
extern ext_t *extensions;
extern const unsigned int num_extensions;

#endif /* EXT_H_ */
