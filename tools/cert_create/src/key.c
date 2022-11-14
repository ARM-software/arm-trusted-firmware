/*
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "cert.h"
#include "cmd_opt.h"
#include "debug.h"
#include "key.h"
#include "sha.h"

#define MAX_FILENAME_LEN		1024

key_t *keys;
unsigned int num_keys;

#if !USING_OPENSSL3
/*
 * Create a new key container
 */
int key_new(key_t *key)
{
	/* Create key pair container */
	key->key = EVP_PKEY_new();
	if (key->key == NULL) {
		return 0;
	}

	return 1;
}
#endif

static int key_create_rsa(key_t *key, int key_bits)
{
#if USING_OPENSSL3
	EVP_PKEY *rsa = EVP_RSA_gen(key_bits);
	if (rsa == NULL) {
		printf("Cannot generate RSA key\n");
		return 0;
	}
	key->key = rsa;
	return 1;
#else
	BIGNUM *e;
	RSA *rsa = NULL;

	e = BN_new();
	if (e == NULL) {
		printf("Cannot create RSA exponent\n");
		return 0;
	}

	if (!BN_set_word(e, RSA_F4)) {
		printf("Cannot assign RSA exponent\n");
		goto err2;
	}

	rsa = RSA_new();
	if (rsa == NULL) {
		printf("Cannot create RSA key\n");
		goto err2;
	}

	if (!RSA_generate_key_ex(rsa, key_bits, e, NULL)) {
		printf("Cannot generate RSA key\n");
		goto err;
	}

	if (!EVP_PKEY_assign_RSA(key->key, rsa)) {
		printf("Cannot assign RSA key\n");
		goto err;
	}

	BN_free(e);
	return 1;

err:
	RSA_free(rsa);
err2:
	BN_free(e);
	return 0;
#endif
}

#ifndef OPENSSL_NO_EC
#if USING_OPENSSL3
static int key_create_ecdsa(key_t *key, int key_bits, const char *curve)
{
	EVP_PKEY *ec = EVP_EC_gen(curve);
	if (ec == NULL) {
		printf("Cannot generate EC key\n");
		return 0;
	}

	key->key = ec;
	return 1;
}

static int key_create_ecdsa_nist(key_t *key, int key_bits)
{
	return key_create_ecdsa(key, key_bits, "prime256v1");
}

static int key_create_ecdsa_brainpool_r(key_t *key, int key_bits)
{
	return key_create_ecdsa(key, key_bits, "brainpoolP256r1");
}

static int key_create_ecdsa_brainpool_t(key_t *key, int key_bits)
{
	return key_create_ecdsa(key, key_bits, "brainpoolP256t1");
}
#else
static int key_create_ecdsa(key_t *key, int key_bits, const int curve_id)
{
	EC_KEY *ec;

	ec = EC_KEY_new_by_curve_name(curve_id);
	if (ec == NULL) {
		printf("Cannot create EC key\n");
		return 0;
	}
	if (!EC_KEY_generate_key(ec)) {
		printf("Cannot generate EC key\n");
		goto err;
	}
	EC_KEY_set_flags(ec, EC_PKEY_NO_PARAMETERS);
	EC_KEY_set_asn1_flag(ec, OPENSSL_EC_NAMED_CURVE);
	if (!EVP_PKEY_assign_EC_KEY(key->key, ec)) {
		printf("Cannot assign EC key\n");
		goto err;
	}

	return 1;

err:
	EC_KEY_free(ec);
	return 0;
}

static int key_create_ecdsa_nist(key_t *key, int key_bits)
{
	return key_create_ecdsa(key, key_bits, NID_X9_62_prime256v1);
}

static int key_create_ecdsa_brainpool_r(key_t *key, int key_bits)
{
	return key_create_ecdsa(key, key_bits, NID_brainpoolP256r1);
}

static int key_create_ecdsa_brainpool_t(key_t *key, int key_bits)
{
	return key_create_ecdsa(key, key_bits, NID_brainpoolP256t1);
}
#endif /* USING_OPENSSL3 */
#endif /* OPENSSL_NO_EC */

typedef int (*key_create_fn_t)(key_t *key, int key_bits);
static const key_create_fn_t key_create_fn[KEY_ALG_MAX_NUM] = {
	[KEY_ALG_RSA] = key_create_rsa,
#ifndef OPENSSL_NO_EC
	[KEY_ALG_ECDSA_NIST] = key_create_ecdsa_nist,
	[KEY_ALG_ECDSA_BRAINPOOL_R] = key_create_ecdsa_brainpool_r,
	[KEY_ALG_ECDSA_BRAINPOOL_T] = key_create_ecdsa_brainpool_t,
#endif /* OPENSSL_NO_EC */
};

int key_create(key_t *key, int type, int key_bits)
{
	if (type >= KEY_ALG_MAX_NUM) {
		printf("Invalid key type\n");
		return 0;
	}

	if (key_create_fn[type]) {
		return key_create_fn[type](key, key_bits);
	}

	return 0;
}

int key_load(key_t *key, unsigned int *err_code)
{
	FILE *fp;
	EVP_PKEY *k;

	if (key->fn) {
		/* Load key from file */
		fp = fopen(key->fn, "r");
		if (fp) {
			k = PEM_read_PrivateKey(fp, &key->key, NULL, NULL);
			fclose(fp);
			if (k) {
				*err_code = KEY_ERR_NONE;
				return 1;
			} else {
				ERROR("Cannot load key from %s\n", key->fn);
				*err_code = KEY_ERR_LOAD;
			}
		} else {
			WARN("Cannot open file %s\n", key->fn);
			*err_code = KEY_ERR_OPEN;
		}
	} else {
		WARN("Key filename not specified\n");
		*err_code = KEY_ERR_FILENAME;
	}

	return 0;
}

int key_store(key_t *key)
{
	FILE *fp;

	if (key->fn) {
		fp = fopen(key->fn, "w");
		if (fp) {
			PEM_write_PrivateKey(fp, key->key,
					NULL, NULL, 0, NULL, NULL);
			fclose(fp);
			return 1;
		} else {
			ERROR("Cannot create file %s\n", key->fn);
		}
	} else {
		ERROR("Key filename not specified\n");
	}

	return 0;
}

int key_init(void)
{
	cmd_opt_t cmd_opt;
	key_t *key;
	unsigned int i;

	keys = malloc((num_def_keys * sizeof(def_keys[0]))
#ifdef PDEF_KEYS
		      + (num_pdef_keys * sizeof(pdef_keys[0]))
#endif
		      );

	if (keys == NULL) {
		ERROR("%s:%d Failed to allocate memory.\n", __func__, __LINE__);
		return 1;
	}

	memcpy(&keys[0], &def_keys[0], (num_def_keys * sizeof(def_keys[0])));
#ifdef PDEF_KEYS
	memcpy(&keys[num_def_keys], &pdef_keys[0],
		(num_pdef_keys * sizeof(pdef_keys[0])));

	num_keys = num_def_keys + num_pdef_keys;
#else
	num_keys = num_def_keys;
#endif
		   ;

	for (i = 0; i < num_keys; i++) {
		key = &keys[i];
		if (key->opt != NULL) {
			cmd_opt.long_opt.name = key->opt;
			cmd_opt.long_opt.has_arg = required_argument;
			cmd_opt.long_opt.flag = NULL;
			cmd_opt.long_opt.val = CMD_OPT_KEY;
			cmd_opt.help_msg = key->help_msg;
			cmd_opt_add(&cmd_opt);
		}
	}

	return 0;
}

key_t *key_get_by_opt(const char *opt)
{
	key_t *key;
	unsigned int i;

	/* Sequential search. This is not a performance concern since the number
	 * of keys is bounded and the code runs on a host machine */
	for (i = 0; i < num_keys; i++) {
		key = &keys[i];
		if (0 == strcmp(key->opt, opt)) {
			return key;
		}
	}

	return NULL;
}

void key_cleanup(void)
{
	unsigned int i;

	for (i = 0; i < num_keys; i++) {
		EVP_PKEY_free(keys[i].key);
		if (keys[i].fn != NULL) {
			void *ptr = keys[i].fn;

			free(ptr);
			keys[i].fn = NULL;
		}
	}
	free(keys);
}

