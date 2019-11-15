/*
 * Copyright (c) 2020, Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_encrypted.h>
#include <drivers/io/io_storage.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_encrypted.h>
#include <tools_share/uuid.h>

static uintptr_t backend_dev_handle;
static uintptr_t backend_dev_spec;
static uintptr_t backend_handle;
static uintptr_t backend_image_spec;

static io_dev_info_t enc_dev_info;

/* Encrypted firmware driver functions */
static int enc_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int enc_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			  io_entity_t *entity);
static int enc_file_len(io_entity_t *entity, size_t *length);
static int enc_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read);
static int enc_file_close(io_entity_t *entity);
static int enc_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params);
static int enc_dev_close(io_dev_info_t *dev_info);

static inline int is_valid_header(struct fw_enc_hdr *header)
{
	if (header->magic == ENC_HEADER_MAGIC)
		return 1;
	else
		return 0;
}

static io_type_t device_type_enc(void)
{
	return IO_TYPE_ENCRYPTED;
}

static const io_dev_connector_t enc_dev_connector = {
	.dev_open = enc_dev_open
};

static const io_dev_funcs_t enc_dev_funcs = {
	.type = device_type_enc,
	.open = enc_file_open,
	.seek = NULL,
	.size = enc_file_len,
	.read = enc_file_read,
	.write = NULL,
	.close = enc_file_close,
	.dev_init = enc_dev_init,
	.dev_close = enc_dev_close,
};

static int enc_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);

	enc_dev_info.funcs = &enc_dev_funcs;
	*dev_info = &enc_dev_info;

	return 0;
}

static int enc_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params)
{
	int result;
	unsigned int image_id = (unsigned int)init_params;

	/* Obtain a reference to the image by querying the platform layer */
	result = plat_get_image_source(image_id, &backend_dev_handle,
				       &backend_dev_spec);
	if (result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		return -ENOENT;
	}

	return result;
}

static int enc_dev_close(io_dev_info_t *dev_info)
{
	backend_dev_handle = (uintptr_t)NULL;
	backend_dev_spec = (uintptr_t)NULL;

	return 0;
}

static int enc_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			 io_entity_t *entity)
{
	int result;

	assert(spec != 0);
	assert(entity != NULL);

	backend_image_spec = spec;

	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != 0) {
		WARN("Failed to open backend device (%i)\n", result);
		result = -ENOENT;
	}

	return result;
}

static int enc_file_len(io_entity_t *entity, size_t *length)
{
	int result;

	assert(entity != NULL);
	assert(length != NULL);

	result = io_size(backend_handle, length);
	if (result != 0) {
		WARN("Failed to read blob length (%i)\n", result);
		return -ENOENT;
	}

	/*
	 * Encryption header is attached at the beginning of the encrypted file
	 * and is not considered a part of the payload.
	 */
	if (*length < sizeof(struct fw_enc_hdr))
		return -EIO;

	*length -= sizeof(struct fw_enc_hdr);

	return result;
}

static int enc_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			 size_t *length_read)
{
	int result;
	struct fw_enc_hdr header;
	enum fw_enc_status_t fw_enc_status;
	size_t bytes_read;
	uint8_t key[ENC_MAX_KEY_SIZE];
	size_t key_len = sizeof(key);
	unsigned int key_flags = 0;
	const io_uuid_spec_t *uuid_spec = (io_uuid_spec_t *)backend_image_spec;

	assert(entity != NULL);
	assert(length_read != NULL);

	result = io_read(backend_handle, (uintptr_t)&header, sizeof(header),
			 &bytes_read);
	if (result != 0) {
		WARN("Failed to read encryption header (%i)\n", result);
		return -ENOENT;
	}

	if (!is_valid_header(&header)) {
		WARN("Encryption header check failed.\n");
		return -ENOENT;
	}

	VERBOSE("Encryption header looks OK.\n");
	fw_enc_status = header.flags & FW_ENC_STATUS_FLAG_MASK;

	if ((header.iv_len > ENC_MAX_IV_SIZE) ||
	    (header.tag_len > ENC_MAX_TAG_SIZE)) {
		WARN("Incorrect IV or tag length\n");
		return -ENOENT;
	}

	result = io_read(backend_handle, buffer, length, &bytes_read);
	if (result != 0) {
		WARN("Failed to read encrypted payload (%i)\n", result);
		return -ENOENT;
	}

	*length_read = bytes_read;

	result = plat_get_enc_key_info(fw_enc_status, key, &key_len, &key_flags,
				       (uint8_t *)&uuid_spec->uuid,
				       sizeof(uuid_t));
	if (result != 0) {
		WARN("Failed to obtain encryption key (%i)\n", result);
		return -ENOENT;
	}

	result = crypto_mod_auth_decrypt(header.dec_algo,
					 (void *)buffer, *length_read, key,
					 key_len, key_flags, header.iv,
					 header.iv_len, header.tag,
					 header.tag_len);
	memset(key, 0, key_len);

	if (result != 0) {
		ERROR("File decryption failed (%i)\n", result);
		return -ENOENT;
	}

	return result;
}

static int enc_file_close(io_entity_t *entity)
{
	io_close(backend_handle);

	backend_image_spec = (uintptr_t)NULL;
	entity->info = 0;

	return 0;
}

/* Exported functions */

/* Register the Encrypted Firmware driver with the IO abstraction */
int register_io_dev_enc(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&enc_dev_info);
	if (result == 0)
		*dev_con = &enc_dev_connector;

	return result;
}
