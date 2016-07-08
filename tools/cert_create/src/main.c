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

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>

#include "cert.h"
#include "cmd_opt.h"
#include "debug.h"
#include "ext.h"
#include "key.h"
#include "platform_oid.h"
#include "sha.h"
#include "tbbr/tbb_ext.h"
#include "tbbr/tbb_cert.h"
#include "tbbr/tbb_key.h"

/*
 * Helper macros to simplify the code. This macro assigns the return value of
 * the 'fn' function to 'v' and exits if the value is NULL.
 */
#define CHECK_NULL(v, fn) \
	do { \
		v = fn; \
		if (v == NULL) { \
			ERROR("NULL object at %s:%d\n", __FILE__, __LINE__); \
			exit(1); \
		} \
	} while (0)

/*
 * This macro assigns the NID corresponding to 'oid' to 'v' and exits if the
 * NID is undefined.
 */
#define CHECK_OID(v, oid) \
	do { \
		v = OBJ_txt2nid(oid); \
		if (v == NID_undef) { \
			ERROR("Cannot find TBB extension %s\n", oid); \
			exit(1); \
		} \
	} while (0)

#define MAX_FILENAME_LEN		1024
#define VAL_DAYS			7300
#define ID_TO_BIT_MASK(id)		(1 << id)
#define NUM_ELEM(x)			((sizeof(x)) / (sizeof(x[0])))
#define HELP_OPT_MAX_LEN		128

/* Global options */
static int key_alg;
static int new_keys;
static int save_keys;
static int print_cert;

/* Info messages created in the Makefile */
extern const char build_msg[];
extern const char platform_msg[];


static char *strdup(const char *str)
{
	int n = strlen(str) + 1;
	char *dup = malloc(n);
	if (dup) {
		strcpy(dup, str);
	}
	return dup;
}

static const char *key_algs_str[] = {
	[KEY_ALG_RSA] = "rsa",
#ifndef OPENSSL_NO_EC
	[KEY_ALG_ECDSA] = "ecdsa"
#endif /* OPENSSL_NO_EC */
};

static void print_help(const char *cmd, const struct option *long_opt)
{
	int rem, i = 0;
	const struct option *opt;
	char line[HELP_OPT_MAX_LEN];
	char *p;

	assert(cmd != NULL);
	assert(long_opt != NULL);

	printf("\n\n");
	printf("The certificate generation tool loads the binary images and\n"
	       "optionally the RSA keys, and outputs the key and content\n"
	       "certificates properly signed to implement the chain of trust.\n"
	       "If keys are provided, they must be in PEM format.\n"
	       "Certificates are generated in DER format.\n");
	printf("\n");
	printf("Usage:\n");
	printf("\t%s [OPTIONS]\n\n", cmd);

	printf("Available options:\n");
	i = 0;
	opt = long_opt;
	while (opt->name) {
		p = line;
		rem = HELP_OPT_MAX_LEN;
		if (isalpha(opt->val)) {
			/* Short format */
			sprintf(p, "-%c,", (char)opt->val);
			p += 3;
			rem -= 3;
		}
		snprintf(p, rem, "--%s %s", opt->name,
			 (opt->has_arg == required_argument) ? "<arg>" : "");
		printf("\t%-32s %s\n", line, cmd_opt_get_help_msg(i));
		opt++;
		i++;
	}
	printf("\n");

	exit(0);
}

static int get_key_alg(const char *key_alg_str)
{
	int i;

	for (i = 0 ; i < NUM_ELEM(key_algs_str) ; i++) {
		if (0 == strcmp(key_alg_str, key_algs_str[i])) {
			return i;
		}
	}

	return -1;
}

static void check_cmd_params(void)
{
	cert_t *cert;
	ext_t *ext;
	key_t *key;
	int i, j;

	/* Only save new keys */
	if (save_keys && !new_keys) {
		ERROR("Only new keys can be saved to disk\n");
		exit(1);
	}

	/* Check that all required options have been specified in the
	 * command line */
	for (i = 0; i < num_certs; i++) {
		cert = &certs[i];
		if (cert->fn == NULL) {
			/* Certificate not requested. Skip to the next one */
			continue;
		}

		/* Check that all parameters required to create this certificate
		 * have been specified in the command line */
		for (j = 0; j < cert->num_ext; j++) {
			ext = &extensions[cert->ext[j]];
			switch (ext->type) {
			case EXT_TYPE_NVCOUNTER:
				/* Counter value must be specified */
				if ((!ext->optional) && (ext->arg == NULL)) {
					ERROR("Value for '%s' not specified\n",
					      ext->ln);
					exit(1);
				}
				break;
			case EXT_TYPE_PKEY:
				/* Key filename must be specified */
				key = &keys[ext->attr.key];
				if (!new_keys && key->fn == NULL) {
					ERROR("Key '%s' required by '%s' not "
					      "specified\n", key->desc,
					      cert->cn);
					exit(1);
				}
				break;
			case EXT_TYPE_HASH:
				/*
				 * Binary image must be specified
				 * unless it is explicitly made optional.
				 */
				if ((!ext->optional) && (ext->arg == NULL)) {
					ERROR("Image for '%s' not specified\n",
					      ext->ln);
					exit(1);
				}
				break;
			default:
				ERROR("Unknown extension type '%d' in '%s'\n",
				      ext->type, ext->ln);
				exit(1);
				break;
			}
		}
	}
}

/* Common command line options */
static const cmd_opt_t common_cmd_opt[] = {
	{
		{ "help", no_argument, NULL, 'h' },
		"Print this message and exit"
	},
	{
		{ "key-alg", required_argument, NULL, 'a' },
		"Key algorithm: 'rsa' (default), 'ecdsa'"
	},
	{
		{ "save-keys", no_argument, NULL, 'k' },
		"Save key pairs into files. Filenames must be provided"
	},
	{
		{ "new-keys", no_argument, NULL, 'n' },
		"Generate new key pairs if no key files are provided"
	},
	{
		{ "print-cert", no_argument, NULL, 'p' },
		"Print the certificates in the standard output"
	}
};

int main(int argc, char *argv[])
{
	STACK_OF(X509_EXTENSION) * sk = NULL;
	X509_EXTENSION *cert_ext = NULL;
	ext_t *ext = NULL;
	key_t *key = NULL;
	cert_t *cert = NULL;
	FILE *file = NULL;
	int i, j, ext_nid, nvctr;
	int c, opt_idx = 0;
	const struct option *cmd_opt;
	const char *cur_opt;
	unsigned int err_code;
	unsigned char md[SHA256_DIGEST_LENGTH];
	const EVP_MD *md_info;

	NOTICE("CoT Generation Tool: %s\n", build_msg);
	NOTICE("Target platform: %s\n", platform_msg);

	/* Set default options */
	key_alg = KEY_ALG_RSA;

	/* Add common command line options */
	for (i = 0; i < NUM_ELEM(common_cmd_opt); i++) {
		cmd_opt_add(&common_cmd_opt[i]);
	}

	/* Initialize the certificates */
	if (cert_init() != 0) {
		ERROR("Cannot initialize certificates\n");
		exit(1);
	}

	/* Initialize the keys */
	if (key_init() != 0) {
		ERROR("Cannot initialize keys\n");
		exit(1);
	}

	/* Initialize the new types and register OIDs for the extensions */
	if (ext_init() != 0) {
		ERROR("Cannot initialize TBB extensions\n");
		exit(1);
	}

	/* Get the command line options populated during the initialization */
	cmd_opt = cmd_opt_get_array();

	while (1) {
		/* getopt_long stores the option index here. */
		c = getopt_long(argc, argv, "a:hknp", cmd_opt, &opt_idx);

		/* Detect the end of the options. */
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'a':
			key_alg = get_key_alg(optarg);
			if (key_alg < 0) {
				ERROR("Invalid key algorithm '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'h':
			print_help(argv[0], cmd_opt);
			break;
		case 'k':
			save_keys = 1;
			break;
		case 'n':
			new_keys = 1;
			break;
		case 'p':
			print_cert = 1;
			break;
		case CMD_OPT_EXT:
			cur_opt = cmd_opt_get_name(opt_idx);
			ext = ext_get_by_opt(cur_opt);
			ext->arg = strdup(optarg);
			break;
		case CMD_OPT_KEY:
			cur_opt = cmd_opt_get_name(opt_idx);
			key = key_get_by_opt(cur_opt);
			key->fn = strdup(optarg);
			break;
		case CMD_OPT_CERT:
			cur_opt = cmd_opt_get_name(opt_idx);
			cert = cert_get_by_opt(cur_opt);
			cert->fn = strdup(optarg);
			break;
		case '?':
		default:
			print_help(argv[0], cmd_opt);
			exit(1);
		}
	}

	/* Check command line arguments */
	check_cmd_params();

	/* Indicate SHA256 as image hash algorithm in the certificate
	 * extension */
	md_info = EVP_sha256();

	/* Load private keys from files (or generate new ones) */
	for (i = 0 ; i < num_keys ; i++) {
		/* First try to load the key from disk */
		if (key_load(&keys[i], &err_code)) {
			/* Key loaded successfully */
			continue;
		}

		/* Key not loaded. Check the error code */
		if (err_code == KEY_ERR_MALLOC) {
			/* Cannot allocate memory. Abort. */
			ERROR("Malloc error while loading '%s'\n", keys[i].fn);
			exit(1);
		} else if (err_code == KEY_ERR_LOAD) {
			/* File exists, but it does not contain a valid private
			 * key. Abort. */
			ERROR("Error loading '%s'\n", keys[i].fn);
			exit(1);
		}

		/* File does not exist, could not be opened or no filename was
		 * given */
		if (new_keys) {
			/* Try to create a new key */
			NOTICE("Creating new key for '%s'\n", keys[i].desc);
			if (!key_create(&keys[i], key_alg)) {
				ERROR("Error creating key '%s'\n", keys[i].desc);
				exit(1);
			}
		} else {
			if (err_code == KEY_ERR_OPEN) {
				ERROR("Error opening '%s'\n", keys[i].fn);
			} else {
				ERROR("Key '%s' not specified\n", keys[i].desc);
			}
			exit(1);
		}
	}

	/* Create the certificates */
	for (i = 0 ; i < num_certs ; i++) {

		cert = &certs[i];

		/* Create a new stack of extensions. This stack will be used
		 * to create the certificate */
		CHECK_NULL(sk, sk_X509_EXTENSION_new_null());

		for (j = 0 ; j < cert->num_ext ; j++) {

			ext = &extensions[cert->ext[j]];

			/* Get OpenSSL internal ID for this extension */
			CHECK_OID(ext_nid, ext->oid);

			/*
			 * Three types of extensions are currently supported:
			 *     - EXT_TYPE_NVCOUNTER
			 *     - EXT_TYPE_HASH
			 *     - EXT_TYPE_PKEY
			 */
			switch (ext->type) {
			case EXT_TYPE_NVCOUNTER:
				if (ext->arg) {
					nvctr = atoi(ext->arg);
					CHECK_NULL(cert_ext, ext_new_nvcounter(ext_nid,
						EXT_CRIT, nvctr));
				}
				break;
			case EXT_TYPE_HASH:
				if (ext->arg == NULL) {
					if (ext->optional) {
						/* Include a hash filled with zeros */
						memset(md, 0x0, SHA256_DIGEST_LENGTH);
					} else {
						/* Do not include this hash in the certificate */
						break;
					}
				} else {
					/* Calculate the hash of the file */
					if (!sha_file(ext->arg, md)) {
						ERROR("Cannot calculate hash of %s\n",
							ext->arg);
						exit(1);
					}
				}
				CHECK_NULL(cert_ext, ext_new_hash(ext_nid,
						EXT_CRIT, md_info, md,
						SHA256_DIGEST_LENGTH));
				break;
			case EXT_TYPE_PKEY:
				CHECK_NULL(cert_ext, ext_new_key(ext_nid,
					EXT_CRIT, keys[ext->attr.key].key));
				break;
			default:
				ERROR("Unknown extension type '%d' in %s\n",
						ext->type, cert->cn);
				exit(1);
			}

			/* Push the extension into the stack */
			sk_X509_EXTENSION_push(sk, cert_ext);
		}

		/* Create certificate. Signed with ROT key */
		if (cert->fn && !cert_new(cert, VAL_DAYS, 0, sk)) {
			ERROR("Cannot create %s\n", cert->cn);
			exit(1);
		}

		sk_X509_EXTENSION_free(sk);
	}


	/* Print the certificates */
	if (print_cert) {
		for (i = 0 ; i < num_certs ; i++) {
			if (!certs[i].x) {
				continue;
			}
			printf("\n\n=====================================\n\n");
			X509_print_fp(stdout, certs[i].x);
		}
	}

	/* Save created certificates to files */
	for (i = 0 ; i < num_certs ; i++) {
		if (certs[i].x && certs[i].fn) {
			file = fopen(certs[i].fn, "w");
			if (file != NULL) {
				i2d_X509_fp(file, certs[i].x);
				fclose(file);
			} else {
				ERROR("Cannot create file %s\n", certs[i].fn);
			}
		}
	}

	/* Save keys */
	if (save_keys) {
		for (i = 0 ; i < num_keys ; i++) {
			if (!key_store(&keys[i])) {
				ERROR("Cannot save %s\n", keys[i].desc);
			}
		}
	}

#ifndef OPENSSL_NO_ENGINE
	ENGINE_cleanup();
#endif
	CRYPTO_cleanup_all_ex_data();

	return 0;
}
