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

/* Files */
enum {
	/* Image file names (inputs) */
	BL2_ID = 0,
	BL30_ID,
	BL31_ID,
	BL32_ID,
	BL33_ID,
	/* Certificate file names (outputs) */
	BL2_CERT_ID,
	TRUSTED_KEY_CERT_ID,
	BL30_KEY_CERT_ID,
	BL30_CERT_ID,
	BL31_KEY_CERT_ID,
	BL31_CERT_ID,
	BL32_KEY_CERT_ID,
	BL32_CERT_ID,
	BL33_KEY_CERT_ID,
	BL33_CERT_ID,
	/* Key file names (input/output) */
	ROT_KEY_ID,
	TRUSTED_WORLD_KEY_ID,
	NON_TRUSTED_WORLD_KEY_ID,
	BL30_KEY_ID,
	BL31_KEY_ID,
	BL32_KEY_ID,
	BL33_KEY_ID,
	NUM_OPTS
};

/* Global options */
static int key_alg;
static int new_keys;
static int save_keys;
static int print_cert;
static int bl30_present;
static int bl32_present;

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

/* Command line options */
static const struct option long_opt[] = {
	/* Binary images */
	{"bl2", required_argument, 0, BL2_ID},
	{"bl30", required_argument, 0, BL30_ID},
	{"bl31", required_argument, 0, BL31_ID},
	{"bl32", required_argument, 0, BL32_ID},
	{"bl33", required_argument, 0, BL33_ID},
	/* Certificate files */
	{"bl2-cert", required_argument, 0, BL2_CERT_ID},
	{"trusted-key-cert", required_argument, 0, TRUSTED_KEY_CERT_ID},
	{"bl30-key-cert", required_argument, 0, BL30_KEY_CERT_ID},
	{"bl30-cert", required_argument, 0, BL30_CERT_ID},
	{"bl31-key-cert", required_argument, 0, BL31_KEY_CERT_ID},
	{"bl31-cert", required_argument, 0, BL31_CERT_ID},
	{"bl32-key-cert", required_argument, 0, BL32_KEY_CERT_ID},
	{"bl32-cert", required_argument, 0, BL32_CERT_ID},
	{"bl33-key-cert", required_argument, 0, BL33_KEY_CERT_ID},
	{"bl33-cert", required_argument, 0, BL33_CERT_ID},
	/* Private key files */
	{"rot-key", required_argument, 0, ROT_KEY_ID},
	{"trusted-world-key", required_argument, 0, TRUSTED_WORLD_KEY_ID},
	{"non-trusted-world-key", required_argument, 0, NON_TRUSTED_WORLD_KEY_ID},
	{"bl30-key", required_argument, 0, BL30_KEY_ID},
	{"bl31-key", required_argument, 0, BL31_KEY_ID},
	{"bl32-key", required_argument, 0, BL32_KEY_ID},
	{"bl33-key", required_argument, 0, BL33_KEY_ID},
	/* Common options */
	{"key-alg", required_argument, 0, 'a'},
	{"help", no_argument, 0, 'h'},
	{"save-keys", no_argument, 0, 'k'},
	{"new-chain", no_argument, 0, 'n'},
	{"print-cert", no_argument, 0, 'p'},
	{0, 0, 0, 0}
};

static void print_help(const char *cmd)
{
	int i = 0;
	printf("\n\n");
	printf("The certificate generation tool loads the binary images and\n"
	       "optionally the RSA keys, and outputs the key and content\n"
	       "certificates properly signed to implement the chain of trust.\n"
	       "If keys are provided, they must be in PEM format.\n"
	       "Certificates are generated in DER format.\n");
	printf("\n");
	printf("Usage:\n\n");
	printf("    %s [-hknp] \\\n", cmd);
	for (i = 0; i < NUM_OPTS; i++) {
		printf("        --%s <file>  \\\n", long_opt[i].name);
	}
	printf("\n");
	printf("-a    Key algorithm: rsa (default), ecdsa\n");
	printf("-h    Print help and exit\n");
	printf("-k    Save key pairs into files. Filenames must be provided\n");
	printf("-n    Generate new key pairs if no key files are provided\n");
	printf("-p    Print the certificates in the standard output\n");
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
	/* Only save new keys */
	if (save_keys && !new_keys) {
		ERROR("Only new keys can be saved to disk\n");
		exit(1);
	}

	/* BL2, BL31 and BL33 are mandatory */
	if (extensions[BL2_HASH_EXT].data.fn == NULL) {
		ERROR("BL2 image not specified\n");
		exit(1);
	}

	if (extensions[BL31_HASH_EXT].data.fn == NULL) {
		ERROR("BL31 image not specified\n");
		exit(1);
	}

	if (extensions[BL33_HASH_EXT].data.fn == NULL) {
		ERROR("BL33 image not specified\n");
		exit(1);
	}

	/* BL30 and BL32 are optional */
	if (extensions[BL30_HASH_EXT].data.fn != NULL) {
		bl30_present = 1;
	}

	if (extensions[BL32_HASH_EXT].data.fn != NULL) {
		bl32_present = 1;
	}

	/* TODO: Certificate filenames */

	/* Filenames to store keys must be specified */
	if (save_keys || !new_keys) {
		if (keys[ROT_KEY].fn == NULL) {
			ERROR("ROT key not specified\n");
			exit(1);
		}

		if (keys[TRUSTED_WORLD_KEY].fn == NULL) {
			ERROR("Trusted World key not specified\n");
			exit(1);
		}

		if (keys[NON_TRUSTED_WORLD_KEY].fn == NULL) {
			ERROR("Non-trusted World key not specified\n");
			exit(1);
		}

		if (keys[BL31_KEY].fn == NULL) {
			ERROR("BL31 key not specified\n");
			exit(1);
		}

		if (keys[BL33_KEY].fn == NULL) {
			ERROR("BL33 key not specified\n");
			exit(1);
		}

		if (bl30_present && (keys[BL30_KEY].fn == NULL)) {
			ERROR("BL30 key not specified\n");
			exit(1);
		}

		if (bl32_present && (keys[BL32_KEY].fn == NULL)) {
			ERROR("BL32 key not specified\n");
			exit(1);
		}
	}
}

int main(int argc, char *argv[])
{
	STACK_OF(X509_EXTENSION) * sk = NULL;
	X509_EXTENSION *cert_ext = NULL;
	ext_t *ext = NULL;
	cert_t *cert;
	FILE *file = NULL;
	int i, j, ext_nid;
	int c, opt_idx = 0;
	unsigned int err_code;
	unsigned char md[SHA256_DIGEST_LENGTH];
	const EVP_MD *md_info;

	NOTICE("CoT Generation Tool: %s\n", build_msg);
	NOTICE("Target platform: %s\n", platform_msg);

	/* Set default options */
	key_alg = KEY_ALG_RSA;

	while (1) {
		/* getopt_long stores the option index here. */
		c = getopt_long(argc, argv, "ahknp", long_opt, &opt_idx);

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
			print_help(argv[0]);
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
		case BL2_ID:
			extensions[BL2_HASH_EXT].data.fn = strdup(optarg);
			break;
		case BL30_ID:
			extensions[BL30_HASH_EXT].data.fn = strdup(optarg);
			break;
		case BL31_ID:
			extensions[BL31_HASH_EXT].data.fn = strdup(optarg);
			break;
		case BL32_ID:
			extensions[BL32_HASH_EXT].data.fn = strdup(optarg);
			break;
		case BL33_ID:
			extensions[BL33_HASH_EXT].data.fn = strdup(optarg);
			break;
		case BL2_CERT_ID:
			certs[BL2_CERT].fn = strdup(optarg);
			break;
		case TRUSTED_KEY_CERT_ID:
			certs[TRUSTED_KEY_CERT].fn = strdup(optarg);
			break;
		case BL30_KEY_CERT_ID:
			certs[BL30_KEY_CERT].fn = strdup(optarg);
			break;
		case BL30_CERT_ID:
			certs[BL30_CERT].fn = strdup(optarg);
			break;
		case BL31_KEY_CERT_ID:
			certs[BL31_KEY_CERT].fn = strdup(optarg);
			break;
		case BL31_CERT_ID:
			certs[BL31_CERT].fn = strdup(optarg);
			break;
		case BL32_KEY_CERT_ID:
			certs[BL32_KEY_CERT].fn = strdup(optarg);
			break;
		case BL32_CERT_ID:
			certs[BL32_CERT].fn = strdup(optarg);
			break;
		case BL33_KEY_CERT_ID:
			certs[BL33_KEY_CERT].fn = strdup(optarg);
			break;
		case BL33_CERT_ID:
			certs[BL33_CERT].fn = strdup(optarg);
			break;
		case ROT_KEY_ID:
			keys[ROT_KEY].fn = strdup(optarg);
			break;
		case TRUSTED_WORLD_KEY_ID:
			keys[TRUSTED_WORLD_KEY].fn = strdup(optarg);
			break;
		case NON_TRUSTED_WORLD_KEY_ID:
			keys[NON_TRUSTED_WORLD_KEY].fn = strdup(optarg);
			break;
		case BL30_KEY_ID:
			keys[BL30_KEY].fn = strdup(optarg);
			break;
		case BL31_KEY_ID:
			keys[BL31_KEY].fn = strdup(optarg);
			break;
		case BL32_KEY_ID:
			keys[BL32_KEY].fn = strdup(optarg);
			break;
		case BL33_KEY_ID:
			keys[BL33_KEY].fn = strdup(optarg);
			break;
		case '?':
		default:
			printf("%s\n", optarg);
			exit(1);
		}
	}

	/* Check command line arguments */
	check_cmd_params();

	/* Register the new types and OIDs for the extensions */
	if (ext_register(extensions) != 0) {
		ERROR("Cannot register TBB extensions\n");
		exit(1);
	}

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
				CHECK_NULL(cert_ext, ext_new_nvcounter(ext_nid,
						EXT_CRIT, ext->data.nvcounter));
				break;
			case EXT_TYPE_HASH:
				if (ext->data.fn == NULL) {
					break;
				}
				if (!sha_file(ext->data.fn, md)) {
					ERROR("Cannot calculate hash of %s\n",
						ext->data.fn);
					exit(1);
				}
				CHECK_NULL(cert_ext, ext_new_hash(ext_nid,
						EXT_CRIT, md_info, md,
						SHA256_DIGEST_LENGTH));
				break;
			case EXT_TYPE_PKEY:
				CHECK_NULL(cert_ext, ext_new_key(ext_nid,
					EXT_CRIT, keys[ext->data.key].key));
				break;
			default:
				ERROR("Unknown extension type in %s\n",
						cert->cn);
				exit(1);
			}

			/* Push the extension into the stack */
			sk_X509_EXTENSION_push(sk, cert_ext);
		}

		/* Create certificate. Signed with ROT key */
		if (!cert_new(cert, VAL_DAYS, 0, sk)) {
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
