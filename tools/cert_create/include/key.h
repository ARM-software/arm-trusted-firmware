/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
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
#ifndef OPENSSL_NO_EC
	KEY_ALG_ECDSA_NIST,
	KEY_ALG_ECDSA_BRAINPOOL_R,
	KEY_ALG_ECDSA_BRAINPOOL_T,
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
#ifndef OPENSSL_NO_EC
	{ 256, 384 },			/* KEY_ALG_ECDSA_NIST */
	{},				/* KEY_ALG_ECDSA_BRAINPOOL_R */
	{}				/* KEY_ALG_ECDSA_BRAINPOOL_T */
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
} cert_key_t;

/* Exported API */
int key_init(void);
cert_key_t *key_get_by_opt(const char *opt);
#if !USING_OPENSSL3
int key_new(cert_key_t *key);
#endif
int key_create(cert_key_t *key, int type, int key_bits);
unsigned int key_load(cert_key_t *key);
int key_store(cert_key_t *key);
void key_cleanup(void);

/* Macro to register the keys used in the CoT */
#define REGISTER_KEYS(_keys) \
	cert_key_t *def_keys = &_keys[0]; \
	const unsigned int num_def_keys = sizeof(_keys)/sizeof(_keys[0])

/* Macro to register the platform defined keys used in the CoT */
#define PLAT_REGISTER_KEYS(_pdef_keys) \
	cert_key_t *pdef_keys = &_pdef_keys[0]; \
	const unsigned int num_pdef_keys = sizeof(_pdef_keys)/sizeof(_pdef_keys[0])

/* Exported variables */
extern cert_key_t *def_keys;
extern const unsigned int num_def_keys;
extern cert_key_t *pdef_keys;
extern const unsigned int num_pdef_keys;

extern cert_key_t *keys;
extern unsigned int num_keys;
#endif /* KEY_H */
