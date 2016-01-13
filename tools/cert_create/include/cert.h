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

#ifndef CERT_H_
#define CERT_H_

#include <openssl/ossl_typ.h>
#include <openssl/x509.h>
#include "ext.h"
#include "key.h"

#define CERT_MAX_EXT			4

/*
 * This structure contains information related to the generation of the
 * certificates. All these fields must be known and specified at build time
 * except for the file name, which is picked up from the command line at
 * run time.
 *
 * One instance of this structure must be created for each of the certificates
 * present in the chain of trust.
 *
 * If the issuer points to this same instance, the generated certificate will
 * be self-signed.
 */
typedef struct cert_s cert_t;
struct cert_s {
	int id;			/* Unique identifier */

	const char *opt;	/* Command line option to pass filename */
	const char *fn;		/* Filename to save the certificate */
	const char *cn;		/* Subject CN (Company Name) */
	const char *help_msg;	/* Help message */

	/* These fields must be defined statically */
	int key;		/* Key to be signed */
	int issuer;		/* Issuer certificate */
	int ext[CERT_MAX_EXT];	/* Certificate extensions */
	int num_ext;		/* Number of extensions in the certificate */

	X509 *x;		/* X509 certificate container */
};

/* Exported API */
int cert_init(void);
cert_t *cert_get_by_opt(const char *opt);
int cert_add_ext(X509 *issuer, X509 *subject, int nid, char *value);
int cert_new(cert_t *cert, int days, int ca, STACK_OF(X509_EXTENSION) * sk);

/* Macro to register the certificates used in the CoT */
#define REGISTER_COT(_certs) \
	cert_t *certs = &_certs[0]; \
	const unsigned int num_certs = sizeof(_certs)/sizeof(_certs[0])

/* Exported variables */
extern cert_t *certs;
extern const unsigned int num_certs;

#endif /* CERT_H_ */
