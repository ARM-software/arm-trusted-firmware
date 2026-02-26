/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <openssl/evp.h>

#include <debug.h>
#include <encrypt.h>
#include <firmware_encrypted.h>

#include "aes_ccm.h"
#include "aes_parameters.h"

#define AES_CCM_KEY_SIZE_BYTES		KEY_SIZE
#define AES_CCM_NONCE_SIZE_BYTES	IV_SIZE
#define AES_CCM_TAG_SIZE_BYTES		TAG_SIZE
#define AES_CCM_KEY_HEX_STRING_SIZE	(2*AES_CCM_KEY_SIZE_BYTES)
#define AES_CCM_NONCE_HEX_STRING_SIZE	(2*AES_CCM_NONCE_SIZE_BYTES)

/*
 * Discounting the tag, the plaintext and ciphertext for CCM mode have
 * the same length.
 */
int ccm_encrypt(unsigned short fw_enc_status, const char *key_string,
		const char *nonce_string, const char *input_file_path,
		const char *output_file_path)
{
	struct stat stat_output;
	uint8_t key[AES_CCM_KEY_SIZE_BYTES];
	uint8_t nonce[AES_CCM_NONCE_SIZE_BYTES];
	uint8_t tag[AES_CCM_TAG_SIZE_BYTES];
	FILE *input_file, *output_file;
	EVP_CIPHER_CTX *ctx;
	int encrypted_data_size;
	int data_size;
	uint8_t *data_buffer;
	size_t bytes_read;
	int ret;

	if (strlen(key_string) != AES_CCM_KEY_HEX_STRING_SIZE) {
		ERROR("Supported key size is %u bytes, got %lu\n",
				AES_CCM_KEY_HEX_STRING_SIZE, strlen(key_string));
		return -1;
	}

	for (int i = 0, j = 0; i < AES_CCM_KEY_SIZE_BYTES; i++, j += 2) {
		if (sscanf(&key_string[j], "%02hhx", &key[i]) != 1) {
			ERROR("Incorrect key format\n");
			return -1;
		}
	}

	if (strlen(nonce_string) != AES_CCM_NONCE_HEX_STRING_SIZE) {
		ERROR("Supported nonce size is %u bytes, got %lu\n",
				AES_CCM_NONCE_HEX_STRING_SIZE, strlen(nonce_string));
		return -1;
	}

	for (int i = 0, j = 0; i < AES_CCM_NONCE_SIZE_BYTES; i++, j += 2) {
		if (sscanf(&nonce_string[j], "%02hhx", &nonce[i]) != 1) {
			ERROR("Incorrect nonce format\n");
			return -1;
		}
	}

	input_file = fopen(input_file_path, "rb");
	if (input_file == NULL) {
		ERROR("Cannot read %s\n", input_file_path);
		return -1;
	}

	ctx = EVP_CIPHER_CTX_new();
	if (ctx == NULL) {
		ERROR("EVP_CIPHER_CTX_new failed\n");
		ret = -1;
		goto out_close_input_file;
	}

	/* Reset cipher */
	ret = EVP_EncryptInit_ex(ctx, EVP_aes_256_ccm(), NULL, NULL, NULL);
	if (ret != 1) {
		ERROR("EVP_EncryptInit_ex failed\n");
		ret = -1;
		goto out_free_evp_ctx;
	}

	ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN,
			AES_CCM_NONCE_SIZE_BYTES, NULL);
	if (ret != 1) {
		ERROR("EVP_CIPHER_CTX_ctrl failed to set IV length\n");
		ret = -1;
		goto out_free_evp_ctx;
	}

	ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG,
			AES_CCM_TAG_SIZE_BYTES, NULL);
	if (ret != 1) {
		ERROR("EVP_CIPHER_CTX_ctrl failed to set tag length\n");
		ret = -1;
		goto out_free_evp_ctx;
	}

	/* Initialize key and IV/nonce */
	ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce);
	if (ret != 1) {
		ERROR("EVP_EncryptInit_ex failed\n");
		ret = -1;
		goto out_free_evp_ctx;
	}

	ret = fstat(fileno(input_file), &stat_output);
	if (ret == -1) {
		ERROR("fstat on input file failed\n");
		goto out_free_evp_ctx;
	}

	/* Plaintext size is the input file's size */
	data_size = stat_output.st_size;

#ifdef BLKGETSIZE64
	if ((stat_output.st_mode & S_IFBLK) != 0) {
		if (ioctl(fileno(input_file), BLKGETSIZE64, &input_file_size) == -1) {
			ERROR("ioctl on input file failed\n");
			goto out_free_evp_ctx;
		}
	}
#endif

	if (data_size <= 0) {
		ERROR("input file size is unexpected\n");
		ret = -1;
		goto out_free_evp_ctx;
	}

	data_buffer = (uint8_t *) malloc(data_size);
	if (data_buffer == NULL) {
		ERROR("input file buffer allocation failed\n");
		ret = -1;
		goto out_free_evp_ctx;
	}

	bytes_read = fread(data_buffer, 1, data_size, input_file);
	if (bytes_read != data_size) {
		ERROR("input file reading failed\n");
		ret = -1;
		goto out_free_data_buffer;
	}

	/* Set CCM's plaintext length */
	ret = EVP_EncryptUpdate(ctx, NULL, &encrypted_data_size,
			NULL, data_size);
	if (ret != 1) {
		ERROR("EVP_EncryptUpdate to set plaintext length failed\n");
		ret = -1;
		goto out_free_data_buffer;
	}

	/* Double checks that OpenSSL has set the length properly */
	assert(encrypted_data_size == data_size);

	/* Encrypt actual plaintext in place, all at once */
	ret = EVP_EncryptUpdate(ctx, data_buffer, &encrypted_data_size,
			data_buffer, data_size);
	if (ret != 1) {
		ERROR("EVP_EncryptUpdate failed\n");
		ret = -1;
		goto out_free_data_buffer;
	}

	/* Double checks that OpenSSL has encrypted all of the plaintext */
	assert(encrypted_data_size == data_size);

	ret = EVP_EncryptFinal_ex(ctx, data_buffer, &encrypted_data_size);
	if (ret != 1) {
		ERROR("EVP_EncryptFinal_ex failed\n");
		ret = -1;
		goto out_free_data_buffer;
	}

	/* Double checks no plaintext was left unencrypted */
	assert(encrypted_data_size == 0);

	ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_CCM_TAG_SIZE_BYTES,
			tag);
	if (ret != 1) {
		ERROR("EVP_CIPHER_CTX_ctrl failed\n");
		ret = -1;
		goto out_free_data_buffer;
	}

	output_file = fopen(output_file_path, "wb");
	if (output_file == NULL) {
		ERROR("Cannot write %s\n", output_file_path);
		goto out_free_data_buffer;
	}

	ret = fseek(output_file, sizeof(struct fw_enc_hdr), SEEK_SET);
	if (ret != 0) {
		ERROR("fseek failed\n");
		goto out_close_output_file;
	}

	size_t bytes_written = fwrite(data_buffer, 1, data_size,
			output_file);
	if (bytes_written != data_size) {
		ERROR("output file writing failed\n");
		ret = -1;
		goto out_close_output_file;
	}

	struct fw_enc_hdr header = {
		.magic = ENC_HEADER_MAGIC,
		.flags = fw_enc_status & FW_ENC_STATUS_FLAG_MASK,
		.dec_algo = KEY_ALG_CCM,
		.iv_len = AES_CCM_NONCE_SIZE_BYTES,
		.tag_len = AES_CCM_TAG_SIZE_BYTES,
	};
	memcpy(header.iv, nonce, AES_CCM_NONCE_SIZE_BYTES);
	memcpy(header.tag, tag, AES_CCM_TAG_SIZE_BYTES);

	ret = fseek(output_file, 0, SEEK_SET);
	if (ret != 0) {
		ERROR("fseek to write header failed\n");
		goto out_close_output_file;
	}

	bytes_written = fwrite(&header, 1, sizeof(struct fw_enc_hdr), output_file);
	if (bytes_written != sizeof(struct fw_enc_hdr)) {
		ERROR("output file writing of header failed\n");
		ret = -1;
	}

out_close_output_file:
	fclose(output_file);

out_free_data_buffer:
	free(data_buffer);

out_free_evp_ctx:
	EVP_CIPHER_CTX_free(ctx);

out_close_input_file:
	fclose(input_file);

	/*
	 * EVP_* APIs return 1 as success but enctool considers
	 * 0 as success.
	 */
	if (ret == 1) {
		ret = 0;
	}

	return ret;
}
