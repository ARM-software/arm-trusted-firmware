/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <openssl/sha.h>
#include <stdio.h>
#include "debug.h"
#include "key.h"

#define BUFFER_SIZE	256

int sha_file(int md_alg, const char *filename, unsigned char *md)
{
	FILE *inFile;
	SHA256_CTX shaContext;
	SHA512_CTX sha512Context;
	int bytes;
	unsigned char data[BUFFER_SIZE];

	if ((filename == NULL) || (md == NULL)) {
		ERROR("%s(): NULL argument\n", __FUNCTION__);
		return 0;
	}

	inFile = fopen(filename, "rb");
	if (inFile == NULL) {
		ERROR("Cannot read %s\n", filename);
		return 0;
	}

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
}
