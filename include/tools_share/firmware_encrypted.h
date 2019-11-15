/*
 * Copyright (c) 2020, Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRMWARE_ENCRYPTED_H
#define FIRMWARE_ENCRYPTED_H

#include <stdint.h>

/* This is used as a signature to validate the encryption header */
#define ENC_HEADER_MAGIC		0xAA640001U

/* Firmware encryption status flag mask */
#define FW_ENC_STATUS_FLAG_MASK		0x1

/*
 * SSK: Secret Symmetric Key
 * BSSK: Binding Secret Symmetric Key
 */
enum fw_enc_status_t {
	FW_ENC_WITH_SSK = 0,
	FW_ENC_WITH_BSSK = 1,
};

#define ENC_MAX_IV_SIZE			16U
#define ENC_MAX_TAG_SIZE		16U
#define ENC_MAX_KEY_SIZE		32U

struct fw_enc_hdr {
	uint32_t magic;
	uint16_t dec_algo;
	uint16_t flags;
	uint16_t iv_len;
	uint16_t tag_len;
	uint8_t iv[ENC_MAX_IV_SIZE];
	uint8_t tag[ENC_MAX_TAG_SIZE];
};

#endif /* FIRMWARE_ENCRYPTED_H */
