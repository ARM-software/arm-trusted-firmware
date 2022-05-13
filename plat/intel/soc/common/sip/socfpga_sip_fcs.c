/*
 * Copyright (c) 2020-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/mmio.h>

#include "socfpga_fcs.h"
#include "socfpga_mailbox.h"
#include "socfpga_sip_svc.h"

/* FCS static variables */
static fcs_crypto_service_aes_data fcs_aes_init_payload;
static fcs_crypto_service_data fcs_sha_get_digest_param;
static fcs_crypto_service_data fcs_sha_mac_verify_param;
static fcs_crypto_service_data fcs_ecdsa_hash_sign_param;
static fcs_crypto_service_data fcs_ecdsa_hash_sig_verify_param;
static fcs_crypto_service_data fcs_sha2_data_sign_param;
static fcs_crypto_service_data fcs_sha2_data_sig_verify_param;
static fcs_crypto_service_data fcs_ecdsa_get_pubkey_param;
static fcs_crypto_service_data fcs_ecdh_request_param;

bool is_size_4_bytes_aligned(uint32_t size)
{
	if ((size % MBOX_WORD_BYTE) != 0U) {
		return false;
	} else {
		return true;
	}
}

static bool is_8_bytes_aligned(uint32_t data)
{
	if ((data % (MBOX_WORD_BYTE * 2U)) != 0U) {
		return false;
	} else {
		return true;
	}
}

static bool is_32_bytes_aligned(uint32_t data)
{
	if ((data % (8U * MBOX_WORD_BYTE)) != 0U) {
		return false;
	} else {
		return true;
	}
}

static int intel_fcs_crypto_service_init(uint32_t session_id,
			uint32_t context_id, uint32_t key_id,
			uint32_t param_size, uint64_t param_data,
			fcs_crypto_service_data *data_addr,
			uint32_t *mbox_error)
{
	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (param_size != 4) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	memset(data_addr, 0, sizeof(fcs_crypto_service_data));

	data_addr->session_id = session_id;
	data_addr->context_id = context_id;
	data_addr->key_id = key_id;
	data_addr->crypto_param_size = param_size;
	data_addr->crypto_param = param_data;

	data_addr->is_updated = 0;

	*mbox_error = 0;

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_random_number_gen(uint64_t addr, uint64_t *ret_size,
					uint32_t *mbox_error)
{
	int status;
	unsigned int i;
	unsigned int resp_len = FCS_RANDOM_WORD_SIZE;
	uint32_t random_data[FCS_RANDOM_WORD_SIZE] = {0U};

	if (!is_address_in_ddr_range(addr, FCS_RANDOM_BYTE_SIZE)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_RANDOM_GEN, NULL, 0U,
			CMD_CASUAL, random_data, &resp_len);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	if (resp_len != FCS_RANDOM_WORD_SIZE) {
		*mbox_error = GENERIC_RESPONSE_ERROR;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*ret_size = FCS_RANDOM_BYTE_SIZE;

	for (i = 0U; i < FCS_RANDOM_WORD_SIZE; i++) {
		mmio_write_32(addr, random_data[i]);
		addr += MBOX_WORD_BYTE;
	}

	flush_dcache_range(addr - *ret_size, *ret_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_random_number_gen_ext(uint32_t session_id, uint32_t context_id,
				uint32_t size, uint32_t *send_id)
{
	int status;
	uint32_t payload_size;
	uint32_t crypto_header;

	if (size > (FCS_RANDOM_EXT_MAX_WORD_SIZE *
		MBOX_WORD_BYTE) || size == 0U) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	crypto_header = (FCS_CS_FIELD_FLAG_INIT | FCS_CS_FIELD_FLAG_FINALIZE) <<
			FCS_CS_FIELD_FLAG_OFFSET;

	fcs_rng_payload payload = {
		session_id,
		context_id,
		crypto_header,
		size
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd_async(send_id, MBOX_FCS_RANDOM_GEN,
					(uint32_t *) &payload, payload_size,
					CMD_INDIRECT);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_send_cert(uint64_t addr, uint64_t size,
					uint32_t *send_id)
{
	int status;

	if (!is_address_in_ddr_range(addr, size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd_async(send_id, MBOX_CMD_VAB_SRC_CERT,
				(uint32_t *)addr, size / MBOX_WORD_BYTE,
				CMD_DIRECT);

	flush_dcache_range(addr, size);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_get_provision_data(uint32_t *send_id)
{
	int status;

	status = mailbox_send_cmd_async(send_id, MBOX_FCS_GET_PROVISION,
				NULL, 0U, CMD_DIRECT);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_cntr_set_preauth(uint8_t counter_type, int32_t counter_value,
					uint32_t test_bit, uint32_t *mbox_error)
{
	int status;
	uint32_t first_word;
	uint32_t payload_size;

	if ((test_bit != MBOX_TEST_BIT) &&
		(test_bit != 0)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if ((counter_type < FCS_BIG_CNTR_SEL) ||
		(counter_type > FCS_SVN_CNTR_3_SEL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if ((counter_type == FCS_BIG_CNTR_SEL) &&
		(counter_value > FCS_BIG_CNTR_VAL_MAX)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if ((counter_type >= FCS_SVN_CNTR_0_SEL) &&
		(counter_type <= FCS_SVN_CNTR_3_SEL) &&
		(counter_value > FCS_SVN_CNTR_VAL_MAX)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	first_word = test_bit | counter_type;
	fcs_cntr_set_preauth_payload payload = {
		first_word,
		counter_value
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;
	status =  mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_CNTR_SET_PREAUTH,
				  (uint32_t *) &payload, payload_size,
				  CMD_CASUAL, NULL, NULL);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_encryption(uint32_t src_addr, uint32_t src_size,
		uint32_t dst_addr, uint32_t dst_size, uint32_t *send_id)
{
	int status;
	uint32_t load_size;

	fcs_encrypt_payload payload = {
		FCS_ENCRYPTION_DATA_0,
		src_addr,
		src_size,
		dst_addr,
		dst_size };
	load_size = sizeof(payload) / MBOX_WORD_BYTE;

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd_async(send_id, MBOX_FCS_ENCRYPT_REQ,
				(uint32_t *) &payload, load_size,
				CMD_INDIRECT);
	inv_dcache_range(dst_addr, dst_size);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_decryption(uint32_t src_addr, uint32_t src_size,
		uint32_t dst_addr, uint32_t dst_size, uint32_t *send_id)
{
	int status;
	uint32_t load_size;
	uintptr_t id_offset;

	id_offset = src_addr + FCS_OWNER_ID_OFFSET;
	fcs_decrypt_payload payload = {
		FCS_DECRYPTION_DATA_0,
		{mmio_read_32(id_offset),
		mmio_read_32(id_offset + MBOX_WORD_BYTE)},
		src_addr,
		src_size,
		dst_addr,
		dst_size };
	load_size = sizeof(payload) / MBOX_WORD_BYTE;

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd_async(send_id, MBOX_FCS_DECRYPT_REQ,
				(uint32_t *) &payload, load_size,
				CMD_INDIRECT);
	inv_dcache_range(dst_addr, dst_size);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_encryption_ext(uint32_t session_id, uint32_t context_id,
		uint32_t src_addr, uint32_t src_size,
		uint32_t dst_addr, uint32_t *dst_size, uint32_t *mbox_error)
{
	int status;
	uint32_t payload_size;
	uint32_t resp_len = FCS_CRYPTION_RESP_WORD_SIZE;
	uint32_t resp_data[FCS_CRYPTION_RESP_WORD_SIZE] = {0U};

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	fcs_encrypt_ext_payload payload = {
		session_id,
		context_id,
		FCS_CRYPTION_CRYPTO_HEADER,
		src_addr,
		src_size,
		dst_addr,
		*dst_size
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_ENCRYPT_REQ,
				(uint32_t *) &payload, payload_size,
				CMD_CASUAL, resp_data, &resp_len);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	if (resp_len != FCS_CRYPTION_RESP_WORD_SIZE) {
		*mbox_error = MBOX_RET_ERROR;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_data[FCS_CRYPTION_RESP_SIZE_OFFSET];
	inv_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_decryption_ext(uint32_t session_id, uint32_t context_id,
		uint32_t src_addr, uint32_t src_size,
		uint32_t dst_addr, uint32_t *dst_size, uint32_t *mbox_error)
{
	int status;
	uintptr_t id_offset;
	uint32_t payload_size;
	uint32_t resp_len = FCS_CRYPTION_RESP_WORD_SIZE;
	uint32_t resp_data[FCS_CRYPTION_RESP_WORD_SIZE] = {0U};

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	id_offset = src_addr + FCS_OWNER_ID_OFFSET;
	fcs_decrypt_ext_payload payload = {
		session_id,
		context_id,
		FCS_CRYPTION_CRYPTO_HEADER,
		{mmio_read_32(id_offset),
		mmio_read_32(id_offset + MBOX_WORD_BYTE)},
		src_addr,
		src_size,
		dst_addr,
		*dst_size
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_DECRYPT_REQ,
				(uint32_t *) &payload, payload_size,
				CMD_CASUAL, resp_data, &resp_len);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	if (resp_len != FCS_CRYPTION_RESP_WORD_SIZE) {
		*mbox_error = MBOX_RET_ERROR;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_data[FCS_CRYPTION_RESP_SIZE_OFFSET];
	inv_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_sigma_teardown(uint32_t session_id, uint32_t *mbox_error)
{
	int status;

	if ((session_id != PSGSIGMA_SESSION_ID_ONE) &&
		(session_id != PSGSIGMA_UNKNOWN_SESSION)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	psgsigma_teardown_msg message = {
		RESERVED_AS_ZERO,
		PSGSIGMA_TEARDOWN_MAGIC,
		session_id
	};

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_PSG_SIGMA_TEARDOWN,
			(uint32_t *) &message, sizeof(message) / MBOX_WORD_BYTE,
			CMD_CASUAL, NULL, NULL);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_chip_id(uint32_t *id_low, uint32_t *id_high, uint32_t *mbox_error)
{
	int status;
	uint32_t load_size;
	uint32_t chip_id[2];

	load_size = sizeof(chip_id) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_GET_CHIPID, NULL,
			0U, CMD_CASUAL, (uint32_t *) chip_id, &load_size);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*id_low = chip_id[0];
	*id_high = chip_id[1];

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_attestation_subkey(uint64_t src_addr, uint32_t src_size,
		uint64_t dst_addr, uint32_t *dst_size, uint32_t *mbox_error)
{
	int status;
	uint32_t send_size = src_size / MBOX_WORD_BYTE;
	uint32_t ret_size = *dst_size / MBOX_WORD_BYTE;


	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_ATTESTATION_SUBKEY,
			(uint32_t *) src_addr, send_size, CMD_CASUAL,
			(uint32_t *) dst_addr, &ret_size);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = ret_size * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_get_measurement(uint64_t src_addr, uint32_t src_size,
		uint64_t dst_addr, uint32_t *dst_size, uint32_t *mbox_error)
{
	int status;
	uint32_t send_size = src_size / MBOX_WORD_BYTE;
	uint32_t ret_size = *dst_size / MBOX_WORD_BYTE;

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_GET_MEASUREMENT,
			(uint32_t *) src_addr, send_size, CMD_CASUAL,
			(uint32_t *) dst_addr, &ret_size);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = ret_size * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_fcs_get_rom_patch_sha384(uint64_t addr, uint64_t *ret_size,
					uint32_t *mbox_error)
{
	int status;
	unsigned int resp_len = FCS_SHA384_WORD_SIZE;

	if (!is_address_in_ddr_range(addr, FCS_SHA384_BYTE_SIZE)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_GET_ROM_PATCH_SHA384, NULL, 0U,
			CMD_CASUAL, (uint32_t *) addr, &resp_len);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	if (resp_len != FCS_SHA384_WORD_SIZE) {
		*mbox_error = GENERIC_RESPONSE_ERROR;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*ret_size = FCS_SHA384_BYTE_SIZE;

	flush_dcache_range(addr, *ret_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_get_attestation_cert(uint32_t cert_request, uint64_t dst_addr,
			uint32_t *dst_size, uint32_t *mbox_error)
{
	int status;
	uint32_t ret_size = *dst_size / MBOX_WORD_BYTE;

	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (cert_request < FCS_ATTEST_FIRMWARE_CERT ||
		cert_request > FCS_ATTEST_CERT_MAX_REQ_PARAM) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_GET_ATTESTATION_CERT,
			(uint32_t *) &cert_request, 1U, CMD_CASUAL,
			(uint32_t *) dst_addr, &ret_size);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = ret_size * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_create_cert_on_reload(uint32_t cert_request,
			uint32_t *mbox_error)
{
	int status;

	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (cert_request < FCS_ATTEST_FIRMWARE_CERT ||
		cert_request > FCS_ATTEST_CERT_MAX_REQ_PARAM) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_CREATE_CERT_ON_RELOAD,
			(uint32_t *) &cert_request, 1U, CMD_CASUAL,
			NULL, NULL);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_open_crypto_service_session(uint32_t *session_id,
			uint32_t *mbox_error)
{
	int status;
	uint32_t resp_len = 1U;

	if ((session_id == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_OPEN_CS_SESSION,
			NULL, 0U, CMD_CASUAL, session_id, &resp_len);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_close_crypto_service_session(uint32_t session_id,
			uint32_t *mbox_error)
{
	int status;

	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_CLOSE_CS_SESSION,
			&session_id, 1U, CMD_CASUAL, NULL, NULL);

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_import_crypto_service_key(uint64_t src_addr, uint32_t src_size,
		uint32_t *send_id)
{
	int status;

	if (src_size > (FCS_CS_KEY_OBJ_MAX_WORD_SIZE *
		MBOX_WORD_BYTE)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	status = mailbox_send_cmd_async(send_id, MBOX_FCS_IMPORT_CS_KEY,
				(uint32_t *)src_addr, src_size / MBOX_WORD_BYTE,
				CMD_INDIRECT);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_export_crypto_service_key(uint32_t session_id, uint32_t key_id,
		uint64_t dst_addr, uint32_t *dst_size,
		uint32_t *mbox_error)
{
	int status;
	uint32_t i;
	uint32_t payload_size;
	uint32_t resp_len = FCS_CS_KEY_OBJ_MAX_WORD_SIZE;
	uint32_t resp_data[FCS_CS_KEY_OBJ_MAX_WORD_SIZE] = {0U};
	uint32_t op_status = 0U;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	fcs_cs_key_payload payload = {
		session_id,
		RESERVED_AS_ZERO,
		RESERVED_AS_ZERO,
		key_id
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_EXPORT_CS_KEY,
			(uint32_t *) &payload, payload_size,
			CMD_CASUAL, resp_data, &resp_len);

	if (resp_len > 0) {
		op_status = resp_data[0] & FCS_CS_KEY_RESP_STATUS_MASK;
	}

	if (status < 0) {
		*mbox_error = (-status) | (op_status << FCS_CS_KEY_RESP_STATUS_OFFSET);
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	if (resp_len > 1) {

		/* Export key object is start at second response data */
		*dst_size = (resp_len - 1) * MBOX_WORD_BYTE;

		for (i = 1U; i < resp_len; i++) {
			mmio_write_32(dst_addr, resp_data[i]);
			dst_addr += MBOX_WORD_BYTE;
		}

		flush_dcache_range(dst_addr - *dst_size, *dst_size);

	} else {

		/* Unexpected response, missing key object in response */
		*mbox_error = MBOX_RET_ERROR;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_remove_crypto_service_key(uint32_t session_id, uint32_t key_id,
		uint32_t *mbox_error)
{
	int status;
	uint32_t payload_size;
	uint32_t resp_len = 1U;
	uint32_t resp_data = 0U;
	uint32_t op_status = 0U;

	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	fcs_cs_key_payload payload = {
		session_id,
		RESERVED_AS_ZERO,
		RESERVED_AS_ZERO,
		key_id
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_REMOVE_CS_KEY,
			(uint32_t *) &payload, payload_size,
			CMD_CASUAL, &resp_data, &resp_len);

	if (resp_len > 0) {
		op_status = resp_data & FCS_CS_KEY_RESP_STATUS_MASK;
	}

	if (status < 0) {
		*mbox_error = (-status) | (op_status << FCS_CS_KEY_RESP_STATUS_OFFSET);
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_get_crypto_service_key_info(uint32_t session_id, uint32_t key_id,
		uint64_t dst_addr, uint32_t *dst_size,
		uint32_t *mbox_error)
{
	int status;
	uint32_t payload_size;
	uint32_t resp_len = FCS_CS_KEY_INFO_MAX_WORD_SIZE;
	uint32_t op_status = 0U;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	fcs_cs_key_payload payload = {
		session_id,
		RESERVED_AS_ZERO,
		RESERVED_AS_ZERO,
		key_id
	};

	payload_size = sizeof(payload) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_GET_CS_KEY_INFO,
				(uint32_t *) &payload, payload_size,
				CMD_CASUAL, (uint32_t *) dst_addr, &resp_len);

	if (resp_len > 0) {
		op_status = mmio_read_32(dst_addr) &
			FCS_CS_KEY_RESP_STATUS_MASK;
	}

	if (status < 0) {
		*mbox_error = (-status) | (op_status << FCS_CS_KEY_RESP_STATUS_OFFSET);
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_get_digest_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint32_t param_size,
				uint64_t param_data, uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_sha_get_digest_param,
				mbox_error);
}

int intel_fcs_get_digest_update_finalize(uint32_t session_id,
				uint32_t context_id, uint32_t src_addr,
				uint32_t src_size, uint64_t dst_addr,
				uint32_t *dst_size, uint8_t is_finalised,
				uint32_t *mbox_error)
{
	int status;
	uint32_t i;
	uint32_t flag;
	uint32_t crypto_header;
	uint32_t resp_len;
	uint32_t payload[FCS_GET_DIGEST_CMD_MAX_WORD_SIZE] = {0U};

	if (dst_size == NULL || mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_sha_get_digest_param.session_id != session_id ||
	    fcs_sha_get_digest_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	/* Source data must be 8 bytes aligned */
	if (!is_8_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		 !is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare crypto header */
	flag = 0;

	if (fcs_sha_get_digest_param.is_updated) {
		fcs_sha_get_digest_param.crypto_param_size = 0;
	} else {
		flag |=  FCS_CS_FIELD_FLAG_INIT;
	}

	if (is_finalised != 0U) {
		flag |=  FCS_CS_FIELD_FLAG_FINALIZE;
	} else {
		flag |=  FCS_CS_FIELD_FLAG_UPDATE;
		fcs_sha_get_digest_param.is_updated = 1;
	}

	crypto_header = ((flag << FCS_CS_FIELD_FLAG_OFFSET) |
			(fcs_sha_get_digest_param.crypto_param_size &
			FCS_CS_FIELD_SIZE_MASK));

	/* Prepare command payload */
	i = 0;
	payload[i] = fcs_sha_get_digest_param.session_id;
	i++;
	payload[i] = fcs_sha_get_digest_param.context_id;
	i++;
	payload[i] = crypto_header;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_INIT) {
		payload[i] = fcs_sha_get_digest_param.key_id;
		i++;
		/* Crypto parameters */
		payload[i] = fcs_sha_get_digest_param.crypto_param
				& INTEL_SIP_SMC_FCS_SHA_MODE_MASK;
		payload[i] |= ((fcs_sha_get_digest_param.crypto_param
				>> INTEL_SIP_SMC_FCS_DIGEST_SIZE_OFFSET)
				& INTEL_SIP_SMC_FCS_DIGEST_SIZE_MASK)
				<< FCS_SHA_HMAC_CRYPTO_PARAM_SIZE_OFFSET;
		i++;
	}
	/* Data source address and size */
	payload[i] = src_addr;
	i++;
	payload[i] = src_size;
	i++;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_GET_DIGEST_REQ,
				payload, i, CMD_CASUAL,
				(uint32_t *) dst_addr, &resp_len);

	if (is_finalised != 0U) {
		memset((void *)&fcs_sha_get_digest_param, 0,
		sizeof(fcs_crypto_service_data));
	}

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_mac_verify_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint32_t param_size,
				uint64_t param_data, uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_sha_mac_verify_param,
				mbox_error);
}

int intel_fcs_mac_verify_update_finalize(uint32_t session_id,
				uint32_t context_id, uint32_t src_addr,
				uint32_t src_size, uint64_t dst_addr,
				uint32_t *dst_size, uint32_t data_size,
				uint8_t is_finalised, uint32_t *mbox_error)
{
	int status;
	uint32_t i;
	uint32_t flag;
	uint32_t crypto_header;
	uint32_t resp_len;
	uint32_t payload[FCS_MAC_VERIFY_CMD_MAX_WORD_SIZE] = {0U};
	uintptr_t mac_offset;

	if (dst_size == NULL || mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_sha_mac_verify_param.session_id != session_id ||
		fcs_sha_mac_verify_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (data_size >= src_size) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(src_size) ||
		!is_8_bytes_aligned(data_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare crypto header */
	flag = 0;

	if (fcs_sha_mac_verify_param.is_updated) {
		fcs_sha_mac_verify_param.crypto_param_size = 0;
	} else {
		flag |=  FCS_CS_FIELD_FLAG_INIT;
	}

	if (is_finalised) {
		flag |=  FCS_CS_FIELD_FLAG_FINALIZE;
	} else {
		flag |=  FCS_CS_FIELD_FLAG_UPDATE;
		fcs_sha_mac_verify_param.is_updated = 1;
	}

	crypto_header = ((flag << FCS_CS_FIELD_FLAG_OFFSET) |
			(fcs_sha_mac_verify_param.crypto_param_size &
			FCS_CS_FIELD_SIZE_MASK));

	/* Prepare command payload */
	i = 0;
	payload[i] = fcs_sha_mac_verify_param.session_id;
	i++;
	payload[i] = fcs_sha_mac_verify_param.context_id;
	i++;
	payload[i] = crypto_header;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_INIT) {
		payload[i] = fcs_sha_mac_verify_param.key_id;
		i++;
		/* Crypto parameters */
		payload[i] = ((fcs_sha_mac_verify_param.crypto_param
				>> INTEL_SIP_SMC_FCS_DIGEST_SIZE_OFFSET)
				& INTEL_SIP_SMC_FCS_DIGEST_SIZE_MASK)
				<< FCS_SHA_HMAC_CRYPTO_PARAM_SIZE_OFFSET;
		i++;
	}
	/* Data source address and size */
	payload[i] = src_addr;
	i++;
	payload[i] = data_size;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_FINALIZE) {
		/* Copy mac data to command */
		mac_offset = src_addr + data_size;
		memcpy((uint8_t *) &payload[i], (uint8_t *) mac_offset,
		src_size - data_size);

		i += (src_size - data_size) / MBOX_WORD_BYTE;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_MAC_VERIFY_REQ,
				payload, i, CMD_CASUAL,
				(uint32_t *) dst_addr, &resp_len);

	if (is_finalised) {
		memset((void *)&fcs_sha_mac_verify_param, 0,
		sizeof(fcs_crypto_service_data));
	}

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_ecdsa_hash_sign_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint32_t param_size,
				uint64_t param_data, uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_ecdsa_hash_sign_param,
				mbox_error);
}

int intel_fcs_ecdsa_hash_sign_finalize(uint32_t session_id, uint32_t context_id,
				uint32_t src_addr, uint32_t src_size,
				uint64_t dst_addr, uint32_t *dst_size,
				uint32_t *mbox_error)
{
	int status;
	uint32_t i;
	uint32_t payload[FCS_ECDSA_HASH_SIGN_CMD_MAX_WORD_SIZE] = {0U};
	uint32_t resp_len;
	uintptr_t hash_data_addr;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_ecdsa_hash_sign_param.session_id != session_id ||
		fcs_ecdsa_hash_sign_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare command payload */
	/* Crypto header */
	i = 0;
	payload[i] = fcs_ecdsa_hash_sign_param.session_id;
	i++;
	payload[i] = fcs_ecdsa_hash_sign_param.context_id;

	i++;
	payload[i] = fcs_ecdsa_hash_sign_param.crypto_param_size
			& FCS_CS_FIELD_SIZE_MASK;
	payload[i] |= (FCS_CS_FIELD_FLAG_INIT | FCS_CS_FIELD_FLAG_UPDATE
			| FCS_CS_FIELD_FLAG_FINALIZE)
			<< FCS_CS_FIELD_FLAG_OFFSET;
	i++;
	payload[i] = fcs_ecdsa_hash_sign_param.key_id;

	/* Crypto parameters */
	i++;
	payload[i] = fcs_ecdsa_hash_sign_param.crypto_param
			& INTEL_SIP_SMC_FCS_ECC_ALGO_MASK;

	/* Hash Data */
	i++;
	hash_data_addr = src_addr;
	memcpy((uint8_t *) &payload[i], (uint8_t *) hash_data_addr,
			src_size);

	i += src_size / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_ECDSA_HASH_SIGN_REQ,
			payload, i, CMD_CASUAL, (uint32_t *) dst_addr,
			&resp_len);

	memset((void *) &fcs_ecdsa_hash_sign_param,
			0, sizeof(fcs_crypto_service_data));

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_ecdsa_hash_sig_verify_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint32_t param_size,
				uint64_t param_data, uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_ecdsa_hash_sig_verify_param,
				mbox_error);
}

int intel_fcs_ecdsa_hash_sig_verify_finalize(uint32_t session_id, uint32_t context_id,
				uint32_t src_addr, uint32_t src_size,
				uint64_t dst_addr, uint32_t *dst_size,
				uint32_t *mbox_error)
{
	int status;
	uint32_t i = 0;
	uint32_t payload[FCS_ECDSA_HASH_SIG_VERIFY_CMD_MAX_WORD_SIZE] = {0U};
	uint32_t resp_len;
	uintptr_t hash_sig_pubkey_addr;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_ecdsa_hash_sig_verify_param.session_id != session_id ||
	fcs_ecdsa_hash_sig_verify_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare command payload */
	/* Crypto header */
	i = 0;
	payload[i] = fcs_ecdsa_hash_sig_verify_param.session_id;

	i++;
	payload[i] = fcs_ecdsa_hash_sig_verify_param.context_id;

	i++;
	payload[i] = fcs_ecdsa_hash_sig_verify_param.crypto_param_size
			& FCS_CS_FIELD_SIZE_MASK;
	payload[i] |= (FCS_CS_FIELD_FLAG_INIT | FCS_CS_FIELD_FLAG_UPDATE
			| FCS_CS_FIELD_FLAG_FINALIZE)
			<< FCS_CS_FIELD_FLAG_OFFSET;

	i++;
	payload[i] = fcs_ecdsa_hash_sig_verify_param.key_id;

	/* Crypto parameters */
	i++;
	payload[i] = fcs_ecdsa_hash_sig_verify_param.crypto_param
			& INTEL_SIP_SMC_FCS_ECC_ALGO_MASK;

	/* Hash Data Word, Signature Data Word and Public Key Data word */
	i++;
	hash_sig_pubkey_addr = src_addr;
	memcpy((uint8_t *) &payload[i],
			(uint8_t *) hash_sig_pubkey_addr, src_size);

	i += (src_size / MBOX_WORD_BYTE);

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_ECDSA_HASH_SIG_VERIFY,
			payload, i, CMD_CASUAL, (uint32_t *) dst_addr,
			&resp_len);

	memset((void *)&fcs_ecdsa_hash_sig_verify_param,
			0, sizeof(fcs_crypto_service_data));

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_ecdsa_sha2_data_sign_init(uint32_t session_id,
				uint32_t context_id, uint32_t key_id,
				uint32_t param_size, uint64_t param_data,
				uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_sha2_data_sign_param,
				mbox_error);
}

int intel_fcs_ecdsa_sha2_data_sign_update_finalize(uint32_t session_id,
				uint32_t context_id, uint32_t src_addr,
				uint32_t src_size, uint64_t dst_addr,
				uint32_t *dst_size, uint8_t is_finalised,
				uint32_t *mbox_error)
{
	int status;
	int i;
	uint32_t flag;
	uint32_t crypto_header;
	uint32_t payload[FCS_ECDSA_SHA2_DATA_SIGN_CMD_MAX_WORD_SIZE] = {0U};
	uint32_t resp_len;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_sha2_data_sign_param.session_id != session_id ||
		fcs_sha2_data_sign_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	/* Source data must be 8 bytes aligned */
	if (!is_8_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare crypto header */
	flag = 0;
	if (fcs_sha2_data_sign_param.is_updated) {
		fcs_sha2_data_sign_param.crypto_param_size = 0;
	} else {
		flag |= FCS_CS_FIELD_FLAG_INIT;
	}

	if (is_finalised != 0U) {
		flag |= FCS_CS_FIELD_FLAG_FINALIZE;
	} else {
		flag |= FCS_CS_FIELD_FLAG_UPDATE;
		fcs_sha2_data_sign_param.is_updated = 1;
	}
	crypto_header = (flag << FCS_CS_FIELD_FLAG_OFFSET) |
			fcs_sha2_data_sign_param.crypto_param_size;

	/* Prepare command payload */
	i = 0;
	payload[i] = fcs_sha2_data_sign_param.session_id;
	i++;
	payload[i] = fcs_sha2_data_sign_param.context_id;
	i++;
	payload[i] = crypto_header;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_INIT) {
		payload[i] = fcs_sha2_data_sign_param.key_id;
		/* Crypto parameters */
		i++;
		payload[i] = fcs_sha2_data_sign_param.crypto_param
				& INTEL_SIP_SMC_FCS_ECC_ALGO_MASK;
		i++;
	}

	/* Data source address and size */
	payload[i] = src_addr;
	i++;
	payload[i] = src_size;
	i++;
	status = mailbox_send_cmd(MBOX_JOB_ID,
			MBOX_FCS_ECDSA_SHA2_DATA_SIGN_REQ, payload,
			i, CMD_CASUAL, (uint32_t *) dst_addr,
			&resp_len);

	if (is_finalised != 0U) {
		memset((void *)&fcs_sha2_data_sign_param, 0,
			sizeof(fcs_crypto_service_data));
	}

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_ecdsa_sha2_data_sig_verify_init(uint32_t session_id,
				uint32_t context_id, uint32_t key_id,
				uint32_t param_size, uint64_t param_data,
				uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_sha2_data_sig_verify_param,
				mbox_error);
}

int intel_fcs_ecdsa_sha2_data_sig_verify_update_finalize(uint32_t session_id,
				uint32_t context_id, uint32_t src_addr,
				uint32_t src_size, uint64_t dst_addr,
				uint32_t *dst_size, uint32_t data_size,
				uint8_t is_finalised, uint32_t *mbox_error)
{
	int status;
	uint32_t i;
	uint32_t flag;
	uint32_t crypto_header;
	uint32_t payload[FCS_ECDSA_SHA2_DATA_SIG_VERIFY_CMD_MAX_WORD_SIZE] = {0U};
	uint32_t resp_len;
	uintptr_t sig_pubkey_offset;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_sha2_data_sig_verify_param.session_id != session_id ||
		fcs_sha2_data_sig_verify_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(src_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_8_bytes_aligned(data_size) ||
		!is_8_bytes_aligned(src_addr)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare crypto header */
	flag = 0;
	if (fcs_sha2_data_sig_verify_param.is_updated)
		fcs_sha2_data_sig_verify_param.crypto_param_size = 0;
	else
		flag |= FCS_CS_FIELD_FLAG_INIT;

	if (is_finalised != 0U)
		flag |= FCS_CS_FIELD_FLAG_FINALIZE;
	else {
		flag |= FCS_CS_FIELD_FLAG_UPDATE;
		fcs_sha2_data_sig_verify_param.is_updated = 1;
	}
	crypto_header = (flag << FCS_CS_FIELD_FLAG_OFFSET) |
			fcs_sha2_data_sig_verify_param.crypto_param_size;

	/* Prepare command payload */
	i = 0;
	payload[i] = fcs_sha2_data_sig_verify_param.session_id;
	i++;
	payload[i] = fcs_sha2_data_sig_verify_param.context_id;
	i++;
	payload[i] = crypto_header;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_INIT) {
		payload[i] = fcs_sha2_data_sig_verify_param.key_id;
		i++;
		/* Crypto parameters */
		payload[i] = fcs_sha2_data_sig_verify_param.crypto_param
				& INTEL_SIP_SMC_FCS_ECC_ALGO_MASK;
		i++;
	}

	/* Data source address and size */
	payload[i] = src_addr;
	i++;
	payload[i] = data_size;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_FINALIZE) {
		/* Signature + Public Key Data */
		sig_pubkey_offset = src_addr + data_size;
		memcpy((uint8_t *) &payload[i], (uint8_t *) sig_pubkey_offset,
			src_size - data_size);

		i += (src_size - data_size) / MBOX_WORD_BYTE;
	}

	status = mailbox_send_cmd(MBOX_JOB_ID,
			MBOX_FCS_ECDSA_SHA2_DATA_SIGN_VERIFY, payload, i,
			CMD_CASUAL, (uint32_t *) dst_addr, &resp_len);

	if (is_finalised != 0U) {
		memset((void *) &fcs_sha2_data_sig_verify_param, 0,
			sizeof(fcs_crypto_service_data));
	}

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_ecdsa_get_pubkey_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint32_t param_size,
				uint64_t param_data, uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_ecdsa_get_pubkey_param,
				mbox_error);
}

int intel_fcs_ecdsa_get_pubkey_finalize(uint32_t session_id, uint32_t context_id,
				uint64_t dst_addr, uint32_t *dst_size,
				uint32_t *mbox_error)
{
	int status;
	int i;
	uint32_t crypto_header;
	uint32_t ret_size;
	uint32_t payload[FCS_ECDSA_GET_PUBKEY_MAX_WORD_SIZE] = {0U};

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_ecdsa_get_pubkey_param.session_id != session_id ||
		fcs_ecdsa_get_pubkey_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	ret_size = *dst_size / MBOX_WORD_BYTE;

	crypto_header = ((FCS_CS_FIELD_FLAG_INIT |
			FCS_CS_FIELD_FLAG_UPDATE |
			FCS_CS_FIELD_FLAG_FINALIZE) <<
			FCS_CS_FIELD_FLAG_OFFSET) |
			fcs_ecdsa_get_pubkey_param.crypto_param_size;
	i = 0;
	/* Prepare command payload */
	payload[i] = session_id;
	i++;
	payload[i] = context_id;
	i++;
	payload[i] = crypto_header;
	i++;
	payload[i] = fcs_ecdsa_get_pubkey_param.key_id;
	i++;
	payload[i] = (uint32_t) fcs_ecdsa_get_pubkey_param.crypto_param &
			INTEL_SIP_SMC_FCS_ECC_ALGO_MASK;
	i++;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_ECDSA_GET_PUBKEY,
			payload, i, CMD_CASUAL,
			(uint32_t *) dst_addr, &ret_size);

	memset((void *) &fcs_ecdsa_get_pubkey_param, 0,
		sizeof(fcs_crypto_service_data));

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = ret_size * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_ecdh_request_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint32_t param_size,
				uint64_t param_data, uint32_t *mbox_error)
{
	return intel_fcs_crypto_service_init(session_id, context_id,
				key_id, param_size, param_data,
				(void *) &fcs_ecdh_request_param,
				mbox_error);
}

int intel_fcs_ecdh_request_finalize(uint32_t session_id, uint32_t context_id,
				uint32_t src_addr, uint32_t src_size,
				uint64_t dst_addr, uint32_t *dst_size,
				uint32_t *mbox_error)
{
	int status;
	uint32_t i;
	uint32_t payload[FCS_ECDH_REQUEST_CMD_MAX_WORD_SIZE] = {0U};
	uint32_t resp_len;
	uintptr_t pubkey;

	if ((dst_size == NULL) || (mbox_error == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (fcs_ecdh_request_param.session_id != session_id ||
		fcs_ecdh_request_param.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(src_addr, src_size) ||
		!is_address_in_ddr_range(dst_addr, *dst_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = *dst_size / MBOX_WORD_BYTE;

	/* Prepare command payload */
	i = 0;
	/* Crypto header */
	payload[i] = fcs_ecdh_request_param.session_id;
	i++;
	payload[i] = fcs_ecdh_request_param.context_id;
	i++;
	payload[i] = fcs_ecdh_request_param.crypto_param_size
			& FCS_CS_FIELD_SIZE_MASK;
	payload[i] |= (FCS_CS_FIELD_FLAG_INIT | FCS_CS_FIELD_FLAG_UPDATE
			| FCS_CS_FIELD_FLAG_FINALIZE)
			<< FCS_CS_FIELD_FLAG_OFFSET;
	i++;
	payload[i] = fcs_ecdh_request_param.key_id;
	i++;
	/* Crypto parameters */
	payload[i] = fcs_ecdh_request_param.crypto_param
			& INTEL_SIP_SMC_FCS_ECC_ALGO_MASK;
	i++;
	/* Public key data */
	pubkey = src_addr;
	memcpy((uint8_t *) &payload[i], (uint8_t *) pubkey, src_size);
	i += src_size / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_FCS_ECDH_REQUEST,
			payload, i, CMD_CASUAL, (uint32_t *) dst_addr,
			&resp_len);

	memset((void *)&fcs_ecdh_request_param, 0,
			sizeof(fcs_crypto_service_data));

	if (status < 0) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*dst_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(dst_addr, *dst_size);

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_aes_crypt_init(uint32_t session_id, uint32_t context_id,
				uint32_t key_id, uint64_t param_addr,
				uint32_t param_size, uint32_t *mbox_error)
{
	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	memset((void *)&fcs_aes_init_payload, 0U, sizeof(fcs_aes_init_payload));

	fcs_aes_init_payload.session_id = session_id;
	fcs_aes_init_payload.context_id = context_id;
	fcs_aes_init_payload.param_size = param_size;
	fcs_aes_init_payload.key_id	= key_id;

	memcpy((uint8_t *) fcs_aes_init_payload.crypto_param,
		(uint8_t *) param_addr, param_size);

	fcs_aes_init_payload.is_updated = 0;

	*mbox_error = 0;

	return INTEL_SIP_SMC_STATUS_OK;
}

int intel_fcs_aes_crypt_update_finalize(uint32_t session_id,
				uint32_t context_id, uint64_t src_addr,
				uint32_t src_size, uint64_t dst_addr,
				uint32_t dst_size, uint8_t is_finalised,
				uint32_t *send_id)
{
	int status;
	int i;
	uint32_t flag;
	uint32_t crypto_header;
	uint32_t fcs_aes_crypt_payload[FCS_AES_CMD_MAX_WORD_SIZE];

	if (fcs_aes_init_payload.session_id != session_id ||
		fcs_aes_init_payload.context_id != context_id) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if ((!is_8_bytes_aligned(src_addr)) ||
		(!is_32_bytes_aligned(src_size)) ||
		(!is_address_in_ddr_range(src_addr, src_size))) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if ((!is_8_bytes_aligned(dst_addr)) ||
		(!is_32_bytes_aligned(dst_size))) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if ((dst_size > FCS_AES_MAX_DATA_SIZE ||
		dst_size < FCS_AES_MIN_DATA_SIZE) ||
		(src_size > FCS_AES_MAX_DATA_SIZE ||
		src_size < FCS_AES_MIN_DATA_SIZE)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	/* Prepare crypto header*/
	flag = 0;
	if (fcs_aes_init_payload.is_updated) {
		fcs_aes_init_payload.param_size = 0;
	} else {
		flag |= FCS_CS_FIELD_FLAG_INIT;
	}

	if (is_finalised != 0U) {
		flag |= FCS_CS_FIELD_FLAG_FINALIZE;
	} else {
		flag |= FCS_CS_FIELD_FLAG_UPDATE;
		fcs_aes_init_payload.is_updated = 1;
	}
	crypto_header = (flag << FCS_CS_FIELD_FLAG_OFFSET) |
			fcs_aes_init_payload.param_size;

	i = 0U;
	fcs_aes_crypt_payload[i] = session_id;
	i++;
	fcs_aes_crypt_payload[i] = context_id;
	i++;
	fcs_aes_crypt_payload[i] = crypto_header;
	i++;

	if ((crypto_header >> FCS_CS_FIELD_FLAG_OFFSET) &
		FCS_CS_FIELD_FLAG_INIT) {
		fcs_aes_crypt_payload[i] = fcs_aes_init_payload.key_id;
		i++;

		memcpy((uint8_t *) &fcs_aes_crypt_payload[i],
			(uint8_t *) fcs_aes_init_payload.crypto_param,
			fcs_aes_init_payload.param_size);

		i += fcs_aes_init_payload.param_size / MBOX_WORD_BYTE;
	}

	fcs_aes_crypt_payload[i] = (uint32_t) src_addr;
	i++;
	fcs_aes_crypt_payload[i] = src_size;
	i++;
	fcs_aes_crypt_payload[i] = (uint32_t) dst_addr;
	i++;
	fcs_aes_crypt_payload[i] = dst_size;
	i++;

	status = mailbox_send_cmd_async(send_id, MBOX_FCS_AES_CRYPT_REQ,
					fcs_aes_crypt_payload, i,
					CMD_INDIRECT);

	if (is_finalised != 0U) {
		memset((void *)&fcs_aes_init_payload, 0,
			sizeof(fcs_aes_init_payload));
	}

	if (status < 0U) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}
