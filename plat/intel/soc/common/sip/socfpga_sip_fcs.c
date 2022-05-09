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

bool is_size_4_bytes_aligned(uint32_t size)
{
	if ((size % MBOX_WORD_BYTE) != 0U) {
		return false;
	} else {
		return true;
	}
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

int intel_fcs_get_attestation_cert(uint32_t cert_request, uint64_t dst_addr,
			uint32_t *dst_size, uint32_t *mbox_error)
{
	int status;
	uint32_t ret_size = *dst_size / MBOX_WORD_BYTE;

	if (mbox_error == NULL) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (cert_request < FCS_ALIAS_CERT ||
		cert_request >
			(FCS_ALIAS_CERT |
			FCS_DEV_ID_SELF_SIGN_CERT |
			FCS_DEV_ID_ENROLL_CERT |
			FCS_ENROLL_SELF_SIGN_CERT |
			FCS_PLAT_KEY_CERT)) {
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

	if (cert_request < FCS_ALIAS_CERT ||
		cert_request >
			(FCS_ALIAS_CERT |
			FCS_DEV_ID_SELF_SIGN_CERT |
			FCS_DEV_ID_ENROLL_CERT |
			FCS_ENROLL_SELF_SIGN_CERT |
			FCS_PLAT_KEY_CERT)) {
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
