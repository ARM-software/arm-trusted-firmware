/*
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "debug.h"
#include "key.h"
#if USING_OPENSSL3
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#else
#include <openssl/sha.h>
#endif

#define BUFFER_SIZE	256

#if USING_OPENSSL3
static int get_algorithm_nid(int hash_alg)
{
	int nids[] = {NID_sha256, NID_sha384, NID_sha512};
	if (hash_alg < 0 || hash_alg >= sizeof(nids) / sizeof(*nids)) {
		return NID_undef;
	}
	return nids[hash_alg];
}
#endif

int sha_file(int md_alg, const char *filename, unsigned char *md)
{
	FILE *inFile;
	int bytes;
	unsigned char data[BUFFER_SIZE];
#if USING_OPENSSL3
	EVP_MD_CTX *mdctx;
	const EVP_MD *md_type;
	int alg_nid;
	unsigned int total_bytes;
#else
	SHA256_CTX shaContext;
	SHA512_CTX sha512Context;
#endif

	if ((filename == NULL) || (md == NULL)) {
		ERROR("%s(): NULL argument\n", __func__);
		return 0;
	}

	inFile = fopen(filename, "rb");
	if (inFile == NULL) {
		ERROR("Cannot read %s\n", filename);
		return 0;
	}

#if USING_OPENSSL3

	mdctx = EVP_MD_CTX_new();
	if (mdctx == NULL) {
		fclose(inFile);
		ERROR("%s(): Could not create EVP MD context\n", __func__);
		return 0;
	}

	alg_nid = get_algorithm_nid(md_alg);
	if (alg_nid == NID_undef) {
		ERROR("%s(): Invalid hash algorithm\n", __func__);
		goto err;
	}

	md_type = EVP_get_digestbynid(alg_nid);
	if (EVP_DigestInit_ex(mdctx, md_type, NULL) == 0) {
		ERROR("%s(): Could not initialize EVP MD digest\n", __func__);
		goto err;
	}

	while ((bytes = fread(data, 1, BUFFER_SIZE, inFile)) != 0) {
		EVP_DigestUpdate(mdctx, data, bytes);
	}
	EVP_DigestFinal_ex(mdctx, md, &total_bytes);

	fclose(inFile);
	EVP_MD_CTX_free(mdctx);
	return 1;

err:
	fclose(inFile);
	EVP_MD_CTX_free(mdctx);
	return 0;

#else

	if (md_alg == HASH_ALG_SHA384) {
		SHA384_Init(&sha512Context);
		while ((bytes = fread(data, 1, BUFFER_SIZE, inFile)) != 0) {
			SHA384_Update(&sha512Context, data, bytes);
		}
		SHA384_Final(md, &sha512Context);
	} else if (md_alg == HASH_ALG_SHA512) {
		SHA512_Init(&sha512Context);
		while ((bytes = fread(data, 1, BUFFER_SIZE, inFile)) != 0) {
			SHA512_Update(&sha512Context, data, bytes);
		}
		SHA512_Final(md, &sha512Context);
	} else {
		SHA256_Init(&shaContext);
		while ((bytes = fread(data, 1, BUFFER_SIZE, inFile)) != 0) {
			SHA256_Update(&shaContext, data, bytes);
		}
		SHA256_Final(md, &shaContext);
	}

	fclose(inFile);
	return 1;

#endif
}

