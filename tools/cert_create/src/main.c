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
#include "tbb_ext.h"
#include "tbb_cert.h"
#include "tbb_key.h"

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
#define NVCOUNTER_VALUE			0

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
static int new_keys;
static int save_keys;
static int print_cert;
static int bl30_present;
static int bl32_present;

/* We are not checking nvcounters in TF. Include them in the certificates but
 * the value will be set to 0 */
static int tf_nvcounter;
static int non_tf_nvcounter;

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
	printf("-h    Print help and exit\n");
	printf("-k    Save key pairs into files. Filenames must be provided\n");
	printf("-n    Generate new key pairs if no key files are provided\n");
	printf("-p    Print the certificates in the standard output\n");
	printf("\n");

	exit(0);
}

static void check_cmd_params(void)
{
	/* BL2, BL31 and BL33 are mandatory */
	if (certs[BL2_CERT].bin == NULL) {
		ERROR("BL2 image not specified\n");
		exit(1);
	}

	if (certs[BL31_CERT].bin == NULL) {
		ERROR("BL31 image not specified\n");
		exit(1);
	}

	if (certs[BL33_CERT].bin == NULL) {
		ERROR("BL33 image not specified\n");
		exit(1);
	}

	/* BL30 and BL32 are optional */
	if (certs[BL30_CERT].bin != NULL) {
		bl30_present = 1;
	}

	if (certs[BL32_CERT].bin != NULL) {
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
	X509_EXTENSION *hash_ext = NULL;
	X509_EXTENSION *nvctr_ext = NULL;
	X509_EXTENSION *trusted_key_ext = NULL;
	X509_EXTENSION *non_trusted_key_ext = NULL;
	FILE *file = NULL;
	int i, tz_nvctr_nid, ntz_nvctr_nid, hash_nid, pk_nid;
	int c, opt_idx = 0;
	unsigned char md[SHA256_DIGEST_LENGTH];

	NOTICE("CoT Generation Tool: %s\n", build_msg);
	NOTICE("Target platform: %s\n", platform_msg);

	while (1) {
		/* getopt_long stores the option index here. */
		c = getopt_long(argc, argv, "hknp", long_opt, &opt_idx);

		/* Detect the end of the options. */
		if (c == -1) {
			break;
		}

		switch (c) {
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
			certs[BL2_CERT].bin = strdup(optarg);
			break;
		case BL30_ID:
			certs[BL30_CERT].bin = strdup(optarg);
			break;
		case BL31_ID:
			certs[BL31_CERT].bin = strdup(optarg);
			break;
		case BL32_ID:
			certs[BL32_CERT].bin = strdup(optarg);
			break;
		case BL33_ID:
			certs[BL33_CERT].bin = strdup(optarg);
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

	/* Set the value of the NVCounters */
	tf_nvcounter = NVCOUNTER_VALUE;
	non_tf_nvcounter = NVCOUNTER_VALUE;

	/* Check command line arguments */
	check_cmd_params();

	/* Register the new types and OIDs for the extensions */
	if (ext_init(tbb_ext) != 0) {
		ERROR("Cannot initialize TBB extensions\n");
		exit(1);
	}

	/* Get non-volatile counters NIDs */
	CHECK_OID(tz_nvctr_nid, TZ_FW_NVCOUNTER_OID);
	CHECK_OID(ntz_nvctr_nid, NTZ_FW_NVCOUNTER_OID);

	/* Load private keys from files (or generate new ones) */
	if (new_keys) {
		for (i = 0 ; i < NUM_KEYS ; i++) {
			if (!key_new(&keys[i])) {
				ERROR("Error creating %s\n", keys[i].desc);
				exit(1);
			}
		}
	} else {
		for (i = 0 ; i < NUM_KEYS ; i++) {
			if (!key_load(&keys[i])) {
				ERROR("Error loading %s\n", keys[i].desc);
				exit(1);
			}
		}
	}

	/* *********************************************************************
	 * BL2 certificate (Trusted Boot Firmware certificate):
	 *     - Self-signed with OEM ROT private key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - BL2 hash
	 **********************************************************************/
	CHECK_NULL(sk, sk_X509_EXTENSION_new_null());

	/* Add the NVCounter as a critical extension */
	CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
			tf_nvcounter));
	sk_X509_EXTENSION_push(sk, nvctr_ext);

	/* Add hash of BL2 as an extension */
	if (!sha_file(certs[BL2_CERT].bin, md)) {
		ERROR("Cannot calculate the hash of %s\n", certs[BL2_CERT].bin);
		exit(1);
	}
	CHECK_OID(hash_nid, BL2_HASH_OID);
	CHECK_NULL(hash_ext, ext_new_hash(hash_nid, EXT_CRIT, md,
			SHA256_DIGEST_LENGTH));
	sk_X509_EXTENSION_push(sk, hash_ext);

	/* Create certificate. Signed with ROT key */
	if (!cert_new(&certs[BL2_CERT], VAL_DAYS, 0, sk)) {
		ERROR("Cannot create %s\n", certs[BL2_CERT].cn);
		exit(1);
	}
	sk_X509_EXTENSION_free(sk);

	/* *********************************************************************
	 * Trusted Key certificate:
	 *     - Self-signed with OEM ROT private key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - TrustedWorldPK
	 *         - NonTrustedWorldPK
	 **********************************************************************/
	CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
	CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
			tf_nvcounter));
	sk_X509_EXTENSION_push(sk, nvctr_ext);
	CHECK_OID(pk_nid, TZ_WORLD_PK_OID);
	CHECK_NULL(trusted_key_ext, ext_new_key(pk_nid, EXT_CRIT,
			keys[TRUSTED_WORLD_KEY].key));
	sk_X509_EXTENSION_push(sk, trusted_key_ext);
	CHECK_OID(pk_nid, NTZ_WORLD_PK_OID);
	CHECK_NULL(non_trusted_key_ext, ext_new_key(pk_nid, EXT_CRIT,
			keys[NON_TRUSTED_WORLD_KEY].key));
	sk_X509_EXTENSION_push(sk, non_trusted_key_ext);
	if (!cert_new(&certs[TRUSTED_KEY_CERT], VAL_DAYS, 0, sk)) {
		ERROR("Cannot create %s\n", certs[TRUSTED_KEY_CERT].cn);
		exit(1);
	}
	sk_X509_EXTENSION_free(sk);

	/* *********************************************************************
	 * BL30 Key certificate (Trusted SCP Firmware Key certificate):
	 *     - Self-signed with Trusted World key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - SCPFirmwareContentCertPK
	 **********************************************************************/
	if (bl30_present) {
		CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
		CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
				tf_nvcounter));
		sk_X509_EXTENSION_push(sk, nvctr_ext);
		CHECK_OID(pk_nid, BL30_CONTENT_CERT_PK_OID);
		CHECK_NULL(trusted_key_ext, ext_new_key(pk_nid, EXT_CRIT,
				keys[BL30_KEY].key));
		sk_X509_EXTENSION_push(sk, trusted_key_ext);
		if (!cert_new(&certs[BL30_KEY_CERT], VAL_DAYS, 0, sk)) {
			ERROR("Cannot create %s\n", certs[BL30_KEY_CERT].cn);
			exit(1);
		}
		sk_X509_EXTENSION_free(sk);
	}

	/* *********************************************************************
	 * BL30 certificate (SCP Firmware Content certificate):
	 *     - Signed with Trusted World Key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - SCPFirmwareHash
	 **********************************************************************/
	if (bl30_present) {
		CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
		CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
				tf_nvcounter));
		sk_X509_EXTENSION_push(sk, nvctr_ext);

		if (!sha_file(certs[BL30_CERT].bin, md)) {
			ERROR("Cannot calculate the hash of %s\n",
					certs[BL30_CERT].bin);
			exit(1);
		}
		CHECK_OID(hash_nid, BL30_HASH_OID);
		CHECK_NULL(hash_ext, ext_new_hash(hash_nid, EXT_CRIT, md,
				SHA256_DIGEST_LENGTH));
		sk_X509_EXTENSION_push(sk, hash_ext);

		if (!cert_new(&certs[BL30_CERT], VAL_DAYS, 0, sk)) {
			ERROR("Cannot create %s\n", certs[BL30_CERT].cn);
			exit(1);
		}

		sk_X509_EXTENSION_free(sk);
	}

	/* *********************************************************************
	 * BL31 Key certificate (Trusted SoC Firmware Key certificate):
	 *     - Self-signed with Trusted World key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - SoCFirmwareContentCertPK
	 **********************************************************************/
	CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
	CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
			tf_nvcounter));
	sk_X509_EXTENSION_push(sk, nvctr_ext);
	CHECK_OID(pk_nid, BL31_CONTENT_CERT_PK_OID);
	CHECK_NULL(trusted_key_ext, ext_new_key(pk_nid, EXT_CRIT,
			keys[BL31_KEY].key));
	sk_X509_EXTENSION_push(sk, trusted_key_ext);
	if (!cert_new(&certs[BL31_KEY_CERT], VAL_DAYS, 0, sk)) {
		ERROR("Cannot create %s\n", certs[BL31_KEY_CERT].cn);
		exit(1);
	}
	sk_X509_EXTENSION_free(sk);

	/* *********************************************************************
	 * BL31 certificate (SOC Firmware Content certificate):
	 *     - Signed with Trusted World Key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - BL31 hash
	 **********************************************************************/
	CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
	CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
			tf_nvcounter));
	sk_X509_EXTENSION_push(sk, nvctr_ext);

	if (!sha_file(certs[BL31_CERT].bin, md)) {
		ERROR("Cannot calculate the hash of %s\n", certs[BL31_CERT].bin);
		exit(1);
	}
	CHECK_OID(hash_nid, BL31_HASH_OID);
	CHECK_NULL(hash_ext, ext_new_hash(hash_nid, EXT_CRIT, md,
			SHA256_DIGEST_LENGTH));
	sk_X509_EXTENSION_push(sk, hash_ext);

	if (!cert_new(&certs[BL31_CERT], VAL_DAYS, 0, sk)) {
		ERROR("Cannot create %s\n", certs[BL31_CERT].cn);
		exit(1);
	}

	sk_X509_EXTENSION_free(sk);

	/* *********************************************************************
	 * BL32 Key certificate (Trusted OS Firmware Key certificate):
	 *     - Self-signed with Trusted World key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - TrustedOSFirmwareContentCertPK
	 **********************************************************************/
	if (bl32_present) {
		CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
		CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
				tf_nvcounter));
		sk_X509_EXTENSION_push(sk, nvctr_ext);
		CHECK_OID(pk_nid, BL32_CONTENT_CERT_PK_OID);
		CHECK_NULL(trusted_key_ext, ext_new_key(pk_nid, EXT_CRIT,
				keys[BL32_KEY].key));
		sk_X509_EXTENSION_push(sk, trusted_key_ext);
		if (!cert_new(&certs[BL32_KEY_CERT], VAL_DAYS, 0, sk)) {
			ERROR("Cannot create %s\n", certs[BL32_KEY_CERT].cn);
			exit(1);
		}
		sk_X509_EXTENSION_free(sk);
	}

	/* *********************************************************************
	 * BL32 certificate (TrustedOS Firmware Content certificate):
	 *     - Signed with Trusted World Key
	 *     - Extensions:
	 *         - TrustedFirmwareNVCounter (TODO)
	 *         - BL32 hash
	 **********************************************************************/
	if (bl32_present) {
		CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
		CHECK_NULL(nvctr_ext, ext_new_nvcounter(tz_nvctr_nid, EXT_CRIT,
				tf_nvcounter));
		sk_X509_EXTENSION_push(sk, nvctr_ext);

		if (!sha_file(certs[BL32_CERT].bin, md)) {
			ERROR("Cannot calculate the hash of %s\n",
					certs[BL32_CERT].bin);
			exit(1);
		}
		CHECK_OID(hash_nid, BL32_HASH_OID);
		CHECK_NULL(hash_ext, ext_new_hash(hash_nid, EXT_CRIT, md,
				SHA256_DIGEST_LENGTH));
		sk_X509_EXTENSION_push(sk, hash_ext);

		if (!cert_new(&certs[BL32_CERT], VAL_DAYS, 0, sk)) {
			ERROR("Cannot create %s\n", certs[BL32_CERT].cn);
			exit(1);
		}

		sk_X509_EXTENSION_free(sk);
	}

	/* *********************************************************************
	 * BL33 Key certificate (Non Trusted Firmware Key certificate):
	 *     - Self-signed with Non Trusted World key
	 *     - Extensions:
	 *         - NonTrustedFirmwareNVCounter (TODO)
	 *         - NonTrustedFirmwareContentCertPK
	 **********************************************************************/
	CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
	CHECK_NULL(nvctr_ext, ext_new_nvcounter(ntz_nvctr_nid, EXT_CRIT,
			non_tf_nvcounter));
	sk_X509_EXTENSION_push(sk, nvctr_ext);
	CHECK_OID(pk_nid, BL33_CONTENT_CERT_PK_OID);
	CHECK_NULL(non_trusted_key_ext, ext_new_key(pk_nid, EXT_CRIT,
			keys[BL33_KEY].key));
	sk_X509_EXTENSION_push(sk, non_trusted_key_ext);
	if (!cert_new(&certs[BL33_KEY_CERT], VAL_DAYS, 0, sk)) {
		ERROR("Cannot create %s\n", certs[BL33_KEY_CERT].cn);
		exit(1);
	}
	sk_X509_EXTENSION_free(sk);

	/* *********************************************************************
	 * BL33 certificate (Non-Trusted World Content certificate):
	 *     - Signed with Non-Trusted World Key
	 *     - Extensions:
	 *         - NonTrustedFirmwareNVCounter (TODO)
	 *         - BL33 hash
	 **********************************************************************/
	CHECK_NULL(sk, sk_X509_EXTENSION_new_null());
	CHECK_NULL(nvctr_ext, ext_new_nvcounter(ntz_nvctr_nid, EXT_CRIT,
			non_tf_nvcounter));
	sk_X509_EXTENSION_push(sk, nvctr_ext);

	if (!sha_file(certs[BL33_CERT].bin, md)) {
		ERROR("Cannot calculate the hash of %s\n", certs[BL33_CERT].bin);
		exit(1);
	}
	CHECK_OID(hash_nid, BL33_HASH_OID);
	CHECK_NULL(hash_ext, ext_new_hash(hash_nid, EXT_CRIT, md,
			SHA256_DIGEST_LENGTH));
	sk_X509_EXTENSION_push(sk, hash_ext);

	if (!cert_new(&certs[BL33_CERT], VAL_DAYS, 0, sk)) {
		ERROR("Cannot create %s\n", certs[BL33_CERT].cn);
		exit(1);
	}
	sk_X509_EXTENSION_free(sk);

	/* Print the certificates */
	if (print_cert) {
		for (i = 0 ; i < NUM_CERTIFICATES ; i++) {
			if (!certs[i].x) {
				continue;
			}
			printf("\n\n=====================================\n\n");
			X509_print_fp(stdout, certs[i].x);
		}
	}

	/* Save created certificates to files */
	for (i = 0 ; i < NUM_CERTIFICATES ; i++) {
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
		for (i = 0 ; i < NUM_KEYS ; i++) {
			if (!key_store(&keys[i])) {
				ERROR("Cannot save %s\n", keys[i].desc);
			}
		}
	}

	X509_EXTENSION_free(hash_ext);
	X509_EXTENSION_free(nvctr_ext);
	X509_EXTENSION_free(trusted_key_ext);
	X509_EXTENSION_free(non_trusted_key_ext);

#ifndef OPENSSL_NO_ENGINE
	ENGINE_cleanup();
#endif
	CRYPTO_cleanup_all_ex_data();

	return 0;
}
