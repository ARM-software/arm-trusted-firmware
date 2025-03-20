/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/libc/endian.h>

#include <drivers/delay_timer.h>
#include <drivers/tpm/tpm2.h>
#include <drivers/tpm/tpm2_chip.h>
#include <drivers/tpm/tpm2_interface.h>

#define CMD_SIZE_OFFSET	6

#define SINGLE_BYTE	1
#define TWO_BYTES	2
#define FOUR_BYTES	4

static struct interface_ops *interface;

static int tpm_xfer(struct tpm_chip_data *chip_data, const tpm_cmd *send, tpm_cmd *receive)
{
	int ret;

	ret = interface->send(chip_data, send);
	if (ret < 0) {
		return ret;
	}

	ret = interface->receive(chip_data, receive);
	if (ret < 0) {
		return ret;
	}

	return TPM_SUCCESS;
}

int tpm_interface_init(struct tpm_chip_data *chip_data, uint8_t locality)
{
	int err;

	interface = tpm_interface_getops(chip_data, locality);

	err = interface->request_access(chip_data, locality);
	if (err != 0) {
		return err;
	}

	return interface->get_info(chip_data, locality);
}

int tpm_interface_close(struct tpm_chip_data *chip_data, uint8_t locality)
{
	return interface->release_locality(chip_data, locality);
}

static int tpm_update_buffer(tpm_cmd *buf, uint32_t new_data, size_t new_len)
{
	int i, j, start;
	uint32_t command_size;

	union {
		uint8_t var8;
		uint16_t var16;
		uint32_t var32;
		uint8_t array[4];
	} tpm_new_data;

	command_size = be32toh(buf->header.cmd_size);

	if (command_size + new_len > MAX_SIZE_CMDBUF) {
		ERROR("%s: buf size exceeded, increase MAX_SIZE_CMDBUF\n",
			__func__);
		return TPM_INVALID_PARAM;
	}
	/*
	 * Subtract the cmd header size from the current command size
	 * so the data buffer is written to starting at index 0.
	 */
	start = command_size - TPM_HEADER_SIZE;

	/*
	 * The TPM, according to the TCG spec, processes data in BE byte order,
	 * in the case where the Host is LE, htobe correctly handles the byte order.
	 * When updating the buffer, keep in mind to only pass sizeof(new_data) or
	 * the variable type size for the new_len function parameter. This ensures
	 * there is only the possiblility of writing 1, 2, or 4 bytes to the buffer,
	 * and that the correct number of bytes are written to data[i].
	 */
	if (new_len == SINGLE_BYTE) {
		tpm_new_data.var8 = new_data & 0xFF;
	} else if (new_len == TWO_BYTES) {
		tpm_new_data.var16 = htobe16(new_data & 0xFFFF);
	} else if (new_len == FOUR_BYTES) {
		tpm_new_data.var32 = htobe32(new_data);
	} else {
		ERROR("%s: Invalid data length\n", __func__);
		return TPM_INVALID_PARAM;
	}

	for (i = start, j = 0; i < start + new_len; i++, j++) {
		buf->data[i] = tpm_new_data.array[j];
	}
	buf->header.cmd_size = htobe32(command_size + new_len);

	return TPM_SUCCESS;
}


int tpm_startup(struct tpm_chip_data *chip_data, uint16_t mode)
{
	tpm_cmd startup_cmd, startup_response;
	uint32_t tpm_rc;
	int ret;

	memset(&startup_cmd, 0, sizeof(startup_cmd));
	memset(&startup_response, 0, sizeof(startup_response));

	startup_cmd.header.tag = htobe16(TPM_ST_NO_SESSIONS);
	startup_cmd.header.cmd_size = htobe32(sizeof(tpm_cmd_hdr));
	startup_cmd.header.cmd_code = htobe32(TPM_CMD_STARTUP);

	ret = tpm_update_buffer(&startup_cmd, mode, sizeof(mode));
	if (ret < 0) {
		return ret;
	}

	ret = tpm_xfer(chip_data, &startup_cmd, &startup_response);
	if (ret < 0) {
		return ret;
	}

	tpm_rc = be32toh(startup_response.header.cmd_code);
	if (tpm_rc != TPM_RESPONSE_SUCCESS) {
		ERROR("%s: response code contains error = %X\n", __func__, tpm_rc);
		return TPM_ERR_RESPONSE;
	}

	return TPM_SUCCESS;
}

int tpm_pcr_extend(struct tpm_chip_data *chip_data, uint32_t index,
		uint16_t algorithm, const uint8_t *digest,
		uint32_t digest_len)
{
	tpm_cmd pcr_extend_cmd, pcr_extend_response;
	uint32_t tpm_rc;
	int ret;

	memset(&pcr_extend_cmd, 0, sizeof(pcr_extend_cmd));
	memset(&pcr_extend_response, 0, sizeof(pcr_extend_response));

	if (digest == NULL) {
		return TPM_INVALID_PARAM;
	}
	pcr_extend_cmd.header.tag = htobe16(TPM_ST_SESSIONS);
	pcr_extend_cmd.header.cmd_size = htobe32(sizeof(tpm_cmd_hdr));
	pcr_extend_cmd.header.cmd_code = htobe32(TPM_CMD_PCR_EXTEND);

	/* handle (PCR Index)*/
	ret = tpm_update_buffer(&pcr_extend_cmd, index, sizeof(index));
	if (ret < 0) {
		return ret;
	}

	/* authorization size , session handle, nonce size, attributes*/
	ret = tpm_update_buffer(&pcr_extend_cmd, TPM_MIN_AUTH_SIZE, sizeof(uint32_t));
	if (ret < 0) {
		return ret;
	}
	ret = tpm_update_buffer(&pcr_extend_cmd, TPM_RS_PW, sizeof(uint32_t));
	if (ret < 0) {
		return ret;
	}
	ret = tpm_update_buffer(&pcr_extend_cmd, TPM_ZERO_NONCE_SIZE, sizeof(uint16_t));
	if (ret < 0) {
		return ret;
	}
	ret = tpm_update_buffer(&pcr_extend_cmd, TPM_ATTRIBUTES_DISABLE, sizeof(uint8_t));
	if (ret < 0) {
		return ret;
	}

	/* hmac/password size */
	ret = tpm_update_buffer(&pcr_extend_cmd, TPM_ZERO_HMAC_SIZE, sizeof(uint16_t));
	if (ret < 0) {
		return ret;
	}

	/* hashes count */
	ret = tpm_update_buffer(&pcr_extend_cmd, TPM_SINGLE_HASH_COUNT, sizeof(uint32_t));
	if (ret < 0) {
		return ret;
	}

	/* hash algorithm */
	ret = tpm_update_buffer(&pcr_extend_cmd, algorithm, sizeof(algorithm));
	if (ret < 0) {
		return ret;
	}

	/* digest */
	for (int i = 0; i < digest_len; i++) {
		ret = tpm_update_buffer(&pcr_extend_cmd, digest[i], sizeof(uint8_t));
		if (ret < 0) {
			return ret;
		}
	}

	ret = tpm_xfer(chip_data, &pcr_extend_cmd, &pcr_extend_response);
	if (ret < 0) {
		return ret;
	}

	tpm_rc = be32toh(pcr_extend_response.header.cmd_code);
	if (tpm_rc != TPM_RESPONSE_SUCCESS) {
		ERROR("%s: response code contains error = %X\n", __func__, tpm_rc);
		return TPM_ERR_RESPONSE;
	}

	return TPM_SUCCESS;
}
