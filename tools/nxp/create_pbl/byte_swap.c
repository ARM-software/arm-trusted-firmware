/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>

#define NUM_MEM_BLOCK		1
#define FOUR_BYTE_ALIGN		4
#define EIGHT_BYTE_ALIGN	8
#define SIZE_TWO_PBL_CMD	24

#define SUCCESS			 0
#define FAILURE			-1
#define BYTE_SWAP_32(word)	((((word) & 0xff000000) >> 24)|	\
				(((word) & 0x00ff0000) >>  8) |	\
				(((word) & 0x0000ff00) <<  8) |	\
				(((word) & 0x000000ff) << 24))


/*
 * Returns:
 *     SUCCESS, on successful byte swapping.
 *     FAILURE, on failure.
 */
int do_byteswap(FILE *fp)
{
	int bytes = 0;
	uint32_t  upper_byte;
	uint32_t  lower_byte;
	uint32_t  pad = 0U;
	/* Carries number of Padding bytes to be appended to
	 * make file size 8 byte aligned.
	 */
	int append_bytes;
	int ret = FAILURE;

	fseek(fp, 0L, SEEK_END);
	bytes = ftell(fp);

	append_bytes = EIGHT_BYTE_ALIGN - (bytes % EIGHT_BYTE_ALIGN);
	if (append_bytes != 0) {
		if (fwrite(&pad, append_bytes, NUM_MEM_BLOCK, fp)
			!= NUM_MEM_BLOCK) {
			printf("%s: Error in appending padding bytes.\n",
				__func__);
			goto byteswap_err;
		}
		bytes += append_bytes;
	}

	rewind(fp);
	while (bytes > 0) {
		if ((fread(&upper_byte, sizeof(upper_byte), NUM_MEM_BLOCK, fp)
			!= NUM_MEM_BLOCK) && (feof(fp) == 0)) {
			printf("%s: Error reading upper bytes.\n", __func__);
			goto byteswap_err;
		}
		if ((fread(&lower_byte, sizeof(lower_byte), NUM_MEM_BLOCK, fp)
			!= NUM_MEM_BLOCK) && (feof(fp) == 0)) {
			printf("%s: Error reading lower bytes.\n", __func__);
			goto byteswap_err;
		}
		fseek(fp, -8L, SEEK_CUR);
		upper_byte = BYTE_SWAP_32(upper_byte);
		lower_byte = BYTE_SWAP_32(lower_byte);
		if (fwrite(&lower_byte, sizeof(lower_byte), NUM_MEM_BLOCK, fp)
			!= NUM_MEM_BLOCK) {
			printf("%s: Error writing lower bytes.\n", __func__);
			goto byteswap_err;
		}
		if (fwrite(&upper_byte, sizeof(upper_byte), NUM_MEM_BLOCK, fp)
			!= NUM_MEM_BLOCK) {
			printf("%s: Error writing upper bytes.\n", __func__);
			goto byteswap_err;
		}
		bytes -= EIGHT_BYTE_ALIGN;
	}
	ret = SUCCESS;

byteswap_err:
	return ret;
}

int main(int argc, char **argv)
{
	FILE *fp = NULL;
	int ret = 0;

	if (argc > 2) {
		printf("Usage format is byteswap <filename>");
		return -1;
	}

	fp = fopen(argv[1], "rb+");
	if (fp == NULL) {
		printf("%s: Error opening the input file: %s\n",
			__func__, argv[1]);
		return -1;
	}

	ret = do_byteswap(fp);
	fclose(fp);
	return ret;
}
