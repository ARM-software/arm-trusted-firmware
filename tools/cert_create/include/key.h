/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef KEY_H
#define KEY_H

#include <openssl/ossl_typ.h>

/* Error codes */
enum {
	KEY_ERR_NONE,
	KEY_ERR_MALLOC,
	KEY_ERR_FILENAME,
	KEY_ERR_OPEN,
	KEY_ERR_LOAD
};

/* Supported key algorithms */
enum {
	KEY_ALG_RSA,		/* RSA PSS as defined by PKCS#1 v2.1 (default) */
	KEY_ALG_RSA_1_5,	/* RSA as defined by PKCS#1 v1.5 */
#ifndef OPENSSL_NO_EC
	KEY_ALG_ECDSA,
#endif /* OPENSSL_NO_EC */
	KEY_ALG_MAX_NUM
};

/* Maximum number of valid key sizes per algorithm */
#define KEY_SIZE_MAX_NUM	4

/* Supported hash algorithms */
enum{
	HASH_ALG_SHA256,
	HASH_ALG_SHA384,
	HASH_ALG_SHA512,
};

/* Supported key sizes */
/* NOTE: the first item in each array is the default key size */
static const unsigned int KEY_SIZES[KEY_ALG_MAX_NUM][KEY_SIZE_MAX_NUM] = {
	{ 2048, 1024, 3072, 4096 },	/* KEY_ALG_RSA */
	{ 2048, 1024, 3072, 4096 },	/* KEY_ALG_RSA_1_5 */
#ifndef OPENSSL_NO_EC
	{}				/* KEY_ALG_ECDSA */
#endif /* OPENSSL_NO_EC */
};

/*
 * This structure contains the relevant information to create the keys
 * required to sign the certificates.
 *
 * One instance of this structure must be created for each key, usually in an
 * array fashion. The filename is obtained at run time from the command line
 * parameters
 */
typedef struct key_s {
	int id;			/* Key id */
	const char *opt;	/* Command line option to specify a key */
	const char *help_msg;	/* Help message */
	const char *desc;	/* Key description (debug purposes) */
	char *fn;		/* Filename to load/store the key */
	EVP_PKEY *key;		/* Key container */
} key_t;

/* Exported API */
int key_init(void);
key_t *key_get_by_opt(const char *opt);
int key_new(key_t *key);
int key_create(key_t *key, int type, int key_bits);
int key_load(key_t *key, unsigned int *err_code);
int key_store(key_t *key);

/* Macro to register the keys used in the CoT */
#define REGISTER_KEYS(_keys) \
	key_t *keys = &_keys[0]; \
	const unsigned int num_keys = sizeof(_keys)/sizeof(_keys[0])

/* Exported variables */
extern key_t *keys;
extern const unsigned int num_keys;

#endif /* KEY_H */
