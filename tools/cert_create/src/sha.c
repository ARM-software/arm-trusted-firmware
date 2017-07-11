/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <openssl/sha.h>
#include <stdio.h>

#include "debug.h"

#define BUFFER_SIZE	256

int sha_file(const char *filename, unsigned char *md)
{
	FILE *inFile;
	SHA256_CTX shaContext;
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

	SHA256_Init(&shaContext);
	while ((bytes = fread(data, 1, BUFFER_SIZE, inFile)) != 0) {
		SHA256_Update(&shaContext, data, bytes);
	}
	SHA256_Final(md, &shaContext);

	fclose(inFile);
	return 1;
}
