/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TPM2_H
#define TPM2_H

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/tpm/tpm2_chip.h>

/* Return values */
enum tpm_ret_value {
	TPM_SUCCESS = 0,
	TPM_ERR_RESPONSE = -1,
	TPM_INVALID_PARAM = -2,
	TPM_ERR_TIMEOUT = -3,
	TPM_ERR_TRANSFER = -4,
};

/*
 * TPM FIFO register space address offsets
 */
#define TPM_FIFO_REG_ACCESS		0x00
#define TPM_FIFO_REG_INTR_ENABLE	0x08
#define TPM_FIFO_REG_INTR_VECTOR	0x0C
#define TPM_FIFO_REG_INTR_STS		0x10
#define TPM_FIFO_REG_INTF_CAPS		0x14
#define TPM_FIFO_REG_STATUS		0x18
#define TPM_FIFO_REG_BURST_COUNT_LO	0x19
#define TPM_FIFO_REG_BURST_COUNT_HI	0x20
#define TPM_FIFO_REG_DATA_FIFO		0x24
#define TPM_FIFO_REG_VENDID		0xF00
#define TPM_FIFO_REG_DEVID		0xF02
#define TPM_FIFO_REG_REVID		0xF04

#define TPM_ST_NO_SESSIONS		U(0x8001)
#define TPM_ST_SESSIONS			U(0x8002)

#define TPM_SU_CLEAR			U(0x0000)
#define TPM_SU_STATE			U(0x0001)

#define TPM_MIN_AUTH_SIZE		9
#define TPM_RS_PW			0x40000009
#define TPM_ZERO_NONCE_SIZE		0
#define TPM_ATTRIBUTES_DISABLE		0
#define TPM_ZERO_HMAC_SIZE		0
#define TPM_SINGLE_HASH_COUNT		1


#define TPM_CMD_STARTUP			U(0x0144)
#define TPM_CMD_PCR_READ		U(0x017E)
#define TPM_CMD_PCR_EXTEND		U(0x0182)

#define TPM_RESPONSE_SUCCESS		U(0x0000)

#define TPM_ACCESS_ACTIVE_LOCALITY	U(1 << 5)
#define TPM_ACCESS_VALID		U(1 << 7)
#define TPM_ACCESS_RELINQUISH_LOCALITY	U(1 << 5)
#define TPM_ACCESS_REQUEST_USE		U(1 << 1)
#define TPM_ACCESS_REQUEST_PENDING	U(1 << 2)

#define TPM_STAT_VALID			U(1 << 7)
#define TPM_STAT_COMMAND_READY		U(1 << 6)
#define TPM_STAT_GO			U(1 << 5)
#define TPM_STAT_AVAIL			U(1 << 4)
#define TPM_STAT_EXPECT			U(1 << 3)

#define TPM_READ_HEADER -1

#define TPM_HEADER_SIZE			10
#define MAX_SIZE_CMDBUF			256
#define MAX_CMD_DATA			(MAX_SIZE_CMDBUF - TPM_HEADER_SIZE)

#pragma pack(1)
typedef struct tpm_cmd_hdr {
	uint16_t tag;
	uint32_t cmd_size;
	uint32_t cmd_code;
} tpm_cmd_hdr;

typedef struct tpm_cmd {
	tpm_cmd_hdr header;
	uint8_t data[MAX_CMD_DATA];
} tpm_cmd;
#pragma pack()

int tpm_interface_init(struct tpm_chip_data *chip_data, uint8_t locality);

int tpm_interface_close(struct tpm_chip_data *chip_data, uint8_t locality);

int tpm_startup(struct tpm_chip_data *chip_data, uint16_t mode);

int tpm_pcr_extend(struct tpm_chip_data *chip_data, uint32_t index,
		   uint16_t algorithm, const uint8_t *digest,
		   uint32_t digest_len);

#endif /* TPM2_H */
