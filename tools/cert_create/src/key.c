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
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "cert.h"
#include "debug.h"
#include "key.h"
#include "platform_oid.h"
#include "sha.h"

#define MAX_FILENAME_LEN		1024

/*
 * Create a new key
 */
int key_new(key_t *key)
{
	RSA *rsa = NULL;
	EVP_PKEY *k = NULL;

	/* Create key pair container */
	k = EVP_PKEY_new();
	if (k == NULL) {
		return 0;
	}

	/* Generate a new RSA key */
	rsa = RSA_generate_key(RSA_KEY_BITS, RSA_F4, NULL, NULL);
	if (EVP_PKEY_assign_RSA(k, rsa)) {
		key->key = k;
		return 1;
	} else {
		printf("Cannot assign RSA key\n");
	}

	if (k)
		EVP_PKEY_free(k);
	return 0;
}

int key_load(key_t *key)
{
	FILE *fp = NULL;
	EVP_PKEY *k = NULL;

	/* Create key pair container */
	k = EVP_PKEY_new();
	if (k == NULL) {
		return 0;
	}

	if (key->fn) {
		/* Load key from file */
		fp = fopen(key->fn, "r");
		if (fp) {
			k = PEM_read_PrivateKey(fp, &k, NULL, NULL);
			fclose(fp);
			if (k) {
				key->key = k;
				return 1;
			} else {
				ERROR("Cannot read key from %s\n", key->fn);
			}
		} else {
			ERROR("Cannot open file %s\n", key->fn);
		}
	} else {
		ERROR("Key filename not specified\n");
	}

	if (k)
		EVP_PKEY_free(k);

	return 0;
}

int key_store(key_t *key)
{
	FILE *fp = NULL;

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
