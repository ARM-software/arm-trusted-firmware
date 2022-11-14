/*
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
#include "sha.h"

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
			ERROR("Cannot find extension %s\n", oid); \
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
static int hash_alg;
static int key_size;
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
	[KEY_ALG_ECDSA_NIST] = "ecdsa",
	[KEY_ALG_ECDSA_BRAINPOOL_R] = "ecdsa-brainpool-regular",
	[KEY_ALG_ECDSA_BRAINPOOL_T] = "ecdsa-brainpool-twisted",
#endif /* OPENSSL_NO_EC */
};

static const char *hash_algs_str[] = {
	[HASH_ALG_SHA256] = "sha256",
	[HASH_ALG_SHA384] = "sha384",
	[HASH_ALG_SHA512] = "sha512",
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
	       "optionally the RSA or ECC keys, and outputs the key and content\n"
	       "certificates properly signed to implement the chain of trust.\n"
	       "If keys are provided, they must be in PEM format.\n"
	       "Certificates are generated in DER format.\n");
	printf("\n");
	printf("Usage:\n");
	printf("\t%s [OPTIONS]\n\n", cmd);

	printf("Available options:\n");
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

static int get_key_size(const char *key_size_str)
{
	char *end;
	long key_size;

	key_size = strtol(key_size_str, &end, 10);
	if (*end != '\0')
		return -1;

	return key_size;
}

static int get_hash_alg(const char *hash_alg_str)
{
	int i;

	for (i = 0 ; i < NUM_ELEM(hash_algs_str) ; i++) {
		if (0 == strcmp(hash_alg_str, hash_algs_str[i])) {
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
	bool valid_size;

	/* Only save new keys */
	if (save_keys && !new_keys) {
		ERROR("Only new keys can be saved to disk\n");
		exit(1);
	}

	/* Validate key-size */
	valid_size = false;
	for (i = 0; i < KEY_SIZE_MAX_NUM; i++) {
		if (key_size == KEY_SIZES[key_alg][i]) {
			valid_size = true;
			break;
		}
	}
	if (!valid_size) {
		ERROR("'%d' is not a valid key size for '%s'\n",
				key_size, key_algs_str[key_alg]);
		NOTICE("Valid sizes are: ");
		for (i = 0; i < KEY_SIZE_MAX_NUM &&
				KEY_SIZES[key_alg][i] != 0; i++) {
			printf("%d ", KEY_SIZES[key_alg][i]);
		}
		printf("\n");
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
		"Key algorithm: 'rsa' (default)- RSAPSS scheme as per PKCS#1 v2.1, " \
		"'ecdsa', 'ecdsa-brainpool-regular', 'ecdsa-brainpool-twisted'"
	},
	{
		{ "key-size", required_argument, NULL, 'b' },
		"Key size (for supported algorithms)."
	},
	{
		{ "hash-alg", required_argument, NULL, 's' },
		"Hash algorithm : 'sha256' (default), 'sha384', 'sha512'"
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
	STACK_OF(X509_EXTENSION) * sk;
	X509_EXTENSION *cert_ext = NULL;
	ext_t *ext;
	key_t *key;
	cert_t *cert;
	FILE *file;
	int i, j, ext_nid, nvctr;
	int c, opt_idx = 0;
	const struct option *cmd_opt;
	const char *cur_opt;
	unsigned int err_code;
	unsigned char md[SHA512_DIGEST_LENGTH];
	unsigned int  md_len;
	const EVP_MD *md_info;

	NOTICE("CoT Generation Tool: %s\n", build_msg);
	NOTICE("Target platform: %s\n", platform_msg);

	/* Set default options */
	key_alg = KEY_ALG_RSA;
	hash_alg = HASH_ALG_SHA256;
	key_size = -1;

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
		ERROR("Cannot initialize extensions\n");
		exit(1);
	}

	/* Get the command line options populated during the initialization */
	cmd_opt = cmd_opt_get_array();

	while (1) {
		/* getopt_long stores the option index here. */
		c = getopt_long(argc, argv, "a:b:hknps:", cmd_opt, &opt_idx);

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
		case 'b':
			key_size = get_key_size(optarg);
			if (key_size <= 0) {
				ERROR("Invalid key size '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'h':
			print_help(argv[0], cmd_opt);
			exit(0);
		case 'k':
			save_keys = 1;
			break;
		case 'n':
			new_keys = 1;
			break;
		case 'p':
			print_cert = 1;
			break;
		case 's':
			hash_alg = get_hash_alg(optarg);
			if (hash_alg < 0) {
				ERROR("Invalid hash algorithm '%s'\n", optarg);
				exit(1);
			}
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

	/* Select a reasonable default key-size */
	if (key_size == -1) {
		key_size = KEY_SIZES[key_alg][0];
	}

	/* Check command line arguments */
	check_cmd_params();

	/* Indicate SHA as image hash algorithm in the certificate
	 * extension */
	if (hash_alg == HASH_ALG_SHA384) {
		md_info = EVP_sha384();
		md_len  = SHA384_DIGEST_LENGTH;
	} else if (hash_alg == HASH_ALG_SHA512) {
		md_info = EVP_sha512();
		md_len  = SHA512_DIGEST_LENGTH;
	} else {
		md_info = EVP_sha256();
		md_len  = SHA256_DIGEST_LENGTH;
	}

	/* Load private keys from files (or generate new ones) */
	for (i = 0 ; i < num_keys ; i++) {
#if !USING_OPENSSL3
		if (!key_new(&keys[i])) {
			ERROR("Failed to allocate key container\n");
			exit(1);
		}
#endif

		/* First try to load the key from disk */
		if (key_load(&keys[i], &err_code)) {
			/* Key loaded successfully */
			continue;
		}

		/* Key not loaded. Check the error code */
		if (err_code == KEY_ERR_LOAD) {
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
			if (!key_create(&keys[i], key_alg, key_size)) {
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

		if (cert->fn == NULL) {
			/* Certificate not requested. Skip to the next one */
			continue;
		}

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
				if (ext->optional && ext->arg == NULL) {
					/* Skip this NVCounter */
					continue;
				} else {
					/* Checked by `check_cmd_params` */
					assert(ext->arg != NULL);
					nvctr = atoi(ext->arg);
					CHECK_NULL(cert_ext, ext_new_nvcounter(ext_nid,
						EXT_CRIT, nvctr));
				}
				break;
			case EXT_TYPE_HASH:
				if (ext->arg == NULL) {
					if (ext->optional) {
						/* Include a hash filled with zeros */
						memset(md, 0x0, SHA512_DIGEST_LENGTH);
					} else {
						/* Do not include this hash in the certificate */
						continue;
					}
				} else {
					/* Calculate the hash of the file */
					if (!sha_file(hash_alg, ext->arg, md)) {
						ERROR("Cannot calculate hash of %s\n",
							ext->arg);
						exit(1);
					}
				}
				CHECK_NULL(cert_ext, ext_new_hash(ext_nid,
						EXT_CRIT, md_info, md,
						md_len));
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

		/* Create certificate. Signed with corresponding key */
		if (!cert_new(hash_alg, cert, VAL_DAYS, 0, sk)) {
			ERROR("Cannot create %s\n", cert->cn);
			exit(1);
		}

		for (cert_ext = sk_X509_EXTENSION_pop(sk); cert_ext != NULL;
				cert_ext = sk_X509_EXTENSION_pop(sk)) {
			X509_EXTENSION_free(cert_ext);
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

	/* If we got here, then we must have filled the key array completely.
	 * We can then safely call free on all of the keys in the array
	 */
	key_cleanup();

#ifndef OPENSSL_NO_ENGINE
	ENGINE_cleanup();
#endif
	CRYPTO_cleanup_all_ex_data();


	/* We allocated strings through strdup, so now we have to free them */

	ext_cleanup();

	cert_cleanup();

	return 0;
}
