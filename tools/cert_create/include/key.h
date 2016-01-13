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

#ifndef KEY_H_
#define KEY_H_

#include <openssl/ossl_typ.h>

#define RSA_KEY_BITS		2048

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
	KEY_ALG_RSA,
#ifndef OPENSSL_NO_EC
	KEY_ALG_ECDSA,
#endif /* OPENSSL_NO_EC */
	KEY_ALG_MAX_NUM
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
int key_create(key_t *key, int type);
int key_load(key_t *key, unsigned int *err_code);
int key_store(key_t *key);

/* Macro to register the keys used in the CoT */
#define REGISTER_KEYS(_keys) \
	key_t *keys = &_keys[0]; \
	const unsigned int num_keys = sizeof(_keys)/sizeof(_keys[0])

/* Exported variables */
extern key_t *keys;
extern const unsigned int num_keys;

#endif /* KEY_H_ */
