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

#define LOCALITY_START_ADDRESS(x, y) \
	((uint16_t)(x->address + (0x1000 * y)))

static int tpm2_get_info(struct tpm_chip_data *chip_data, uint8_t locality)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, locality);
	uint32_t vid_did;
	uint8_t revision;
	int err;

	err = tpm2_fifo_read_chunk(tpm_base_addr + TPM_FIFO_REG_VENDID, DWORD, &vid_did);
	if (err < 0) {
		return err;
	}

	err = tpm2_fifo_read_byte(tpm_base_addr + TPM_FIFO_REG_REVID, &revision);
	if (err < 0) {
		return err;
	}

	INFO("TPM Chip: vendor-id 0x%x, device-id 0x%x, revision-id: 0x%x\n",
		0xFFFF & vid_did, vid_did >> 16, revision);

	return TPM_SUCCESS;
}

static int tpm2_wait_reg_bits(uint16_t reg, uint8_t set, unsigned long timeout, uint8_t *status)
{
	int err;
	uint64_t timeout_delay = timeout_init_us(timeout * 1000);

	do {
		err = tpm2_fifo_read_byte(reg, status);
		if (err < 0) {
			return err;
		}
		if ((*status & set) == set) {
			return TPM_SUCCESS;
		}
	} while (!timeout_elapsed(timeout_delay));

	return TPM_ERR_TIMEOUT;
}

static int tpm2_fifo_request_access(struct tpm_chip_data *chip_data, uint8_t locality)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, locality);
	uint8_t status;
	int err;

	err = tpm2_fifo_write_byte(tpm_base_addr + TPM_FIFO_REG_ACCESS, TPM_ACCESS_REQUEST_USE);
	if (err < 0) {
		return err;
	}

	err = tpm2_wait_reg_bits(tpm_base_addr + TPM_FIFO_REG_ACCESS,
				TPM_ACCESS_ACTIVE_LOCALITY,
				chip_data->timeout_msec_a, &status);
	if (err == 0) {
		chip_data->locality = locality;
		return TPM_SUCCESS;
	}

	return err;
}

static int tpm2_fifo_release_locality(struct tpm_chip_data *chip_data, uint8_t locality)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, locality);
	uint8_t buf;
	int err;

	err = tpm2_fifo_read_byte(tpm_base_addr + TPM_FIFO_REG_ACCESS, &buf);
	if (err < 0) {
		return err;
	}

	if (buf & (TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID)) {
		return tpm2_fifo_write_byte(tpm_base_addr + TPM_FIFO_REG_ACCESS,
				TPM_ACCESS_RELINQUISH_LOCALITY);
	}

	ERROR("%s: Unable to release locality\n", __func__);
	return TPM_ERR_RESPONSE;
}

static int tpm2_fifo_prepare(struct tpm_chip_data *chip_data)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, chip_data->locality);
	uint8_t status;
	int err;

	err = tpm2_fifo_write_byte(tpm_base_addr + TPM_FIFO_REG_STATUS, TPM_STAT_COMMAND_READY);
	if (err < 0) {
		return err;
	}

	err = tpm2_wait_reg_bits(tpm_base_addr + TPM_FIFO_REG_STATUS,
				TPM_STAT_COMMAND_READY,
				chip_data->timeout_msec_b, &status);
	if (err < 0) {
		ERROR("%s: TPM Status Busy\n", __func__);
		return err;
	}

	return TPM_SUCCESS;
}

static int tpm2_fifo_get_burstcount(struct tpm_chip_data *chip_data, uint16_t *burstcount)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, chip_data->locality);
	uint64_t timeout_delay = timeout_init_us(chip_data->timeout_msec_a * 1000);
	int err;

	if (burstcount == NULL) {
		return TPM_INVALID_PARAM;
	}

	do {
		uint8_t byte0, byte1;

		err = tpm2_fifo_read_byte(tpm_base_addr + TPM_FIFO_REG_BURST_COUNT_LO, &byte0);
		if (err < 0) {
			return err;
		}

		err = tpm2_fifo_read_byte(tpm_base_addr + TPM_FIFO_REG_BURST_COUNT_HI, &byte1);
		if (err < 0) {
			return err;
		}

		*burstcount = (uint16_t)((byte1 << 8) + byte0);
		if (*burstcount != 0U) {
			return TPM_SUCCESS;
		}
	} while (!timeout_elapsed(timeout_delay));

	return TPM_ERR_TIMEOUT;
}

static int tpm2_fifo_send(struct tpm_chip_data *chip_data, const tpm_cmd *buf)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, chip_data->locality);
	uint8_t status;
	uint16_t burstcnt;
	int err;
	uint32_t len, index;

	if (sizeof(buf->header) != TPM_HEADER_SIZE) {
		ERROR("%s: invalid command header size.\n", __func__);
		return TPM_INVALID_PARAM;
	}

	err = tpm2_fifo_read_byte(tpm_base_addr + TPM_FIFO_REG_STATUS, &status);
	if (err < 0) {
		return err;
	}

	if (!(status & TPM_STAT_COMMAND_READY)) {
		err = tpm2_fifo_prepare(chip_data);
		if (err < 0) {
			return err;
		}
	}

	/* write the command header to the TPM first */
	const uint8_t *header_data = (const uint8_t *)&buf->header;

	for (index = 0; index < TPM_HEADER_SIZE; index++) {
		err = tpm2_fifo_write_byte(tpm_base_addr + TPM_FIFO_REG_DATA_FIFO,
				header_data[index]);
		if (err < 0) {
			return err;
		}
	}

	len =  be32toh(buf->header.cmd_size);

	while (index < len) {
		err = tpm2_fifo_get_burstcount(chip_data, &burstcnt);
		if (err < 0) {
			return err;
		}

		for (; burstcnt > 0U && index < len; burstcnt--) {
			err = tpm2_fifo_write_byte(tpm_base_addr + TPM_FIFO_REG_DATA_FIFO,
					buf->data[index - TPM_HEADER_SIZE]);
			if (err < 0) {
				return err;
			}
			index++;
		}
	}

	err = tpm2_wait_reg_bits(tpm_base_addr + TPM_FIFO_REG_STATUS,
				TPM_STAT_VALID,
				chip_data->timeout_msec_c,
				&status);
	if (err < 0) {
		return err;
	}

	if (status & TPM_STAT_EXPECT) {
		ERROR("%s: TPM is still expecting data after command buffer is sent\n", __func__);
		return TPM_ERR_TRANSFER;
	}

	err = tpm2_fifo_write_byte(tpm_base_addr + TPM_FIFO_REG_STATUS, TPM_STAT_GO);
	if (err < 0) {
		return err;
	}

	return TPM_SUCCESS;
}

static int tpm2_fifo_read_data(struct tpm_chip_data *chip_data, tpm_cmd *buf,
			uint16_t tpm_base_addr, uint8_t *status, int *size, int bytes_expected)
{
	int err, read_size, loop_index;
	uint16_t burstcnt;
	uint8_t *read_data;

	if (bytes_expected == TPM_READ_HEADER) {
		/* read the response header from the TPM first */
		read_data = (uint8_t *)&buf->header;
		read_size = TPM_HEADER_SIZE;
		loop_index = *size;
	} else {
		/* process the rest of the mssg with bytes_expected */
		read_data = buf->data;
		read_size = bytes_expected;
		loop_index = *size - TPM_HEADER_SIZE;
	}

	err = tpm2_wait_reg_bits(tpm_base_addr + TPM_FIFO_REG_STATUS,
				TPM_STAT_AVAIL,
				chip_data->timeout_msec_c,
				status);
	if (err < 0) {
		return err;
	}

	while (*size < read_size) {
		err = tpm2_fifo_get_burstcount(chip_data, &burstcnt);
		if (err < 0) {
			ERROR("%s: TPM burst count error\n", __func__);
			return err;
		}

		for (; burstcnt > 0U && loop_index < read_size;
		    burstcnt--, loop_index++, (*size)++) {
			err = tpm2_fifo_read_byte(
				tpm_base_addr + TPM_FIFO_REG_DATA_FIFO,
				(void *)&read_data[loop_index]);
			if (err < 0) {
				return err;
			}
		}
	}

	return TPM_SUCCESS;
}

static int tpm2_fifo_receive(struct tpm_chip_data *chip_data, tpm_cmd *buf)
{
	uint16_t tpm_base_addr = LOCALITY_START_ADDRESS(chip_data, chip_data->locality);
	int size = 0, bytes_expected, err;
	uint8_t status;

	err = tpm2_fifo_read_data(chip_data, buf, tpm_base_addr, &status, &size, TPM_READ_HEADER);
	if (err < 0) {
		return err;
	}

	bytes_expected = be32toh(buf->header.cmd_size);
	if (bytes_expected > sizeof(*buf)) {
		ERROR("%s: tpm response buffer cannot store expected response\n", __func__);
		return TPM_INVALID_PARAM;
	}

	if (size == bytes_expected) {
		return size;
	}

	err = tpm2_fifo_read_data(chip_data, buf, tpm_base_addr, &status, &size, bytes_expected);
	if (err < 0) {
		return err;
	}

	if (size < bytes_expected) {
		ERROR("%s: response buffer size is less than expected\n", __func__);
		return TPM_ERR_RESPONSE;
	}

	return TPM_SUCCESS;
}

static interface_ops_t fifo_ops = {
	.get_info = tpm2_get_info,
	.send = tpm2_fifo_send,
	.receive = tpm2_fifo_receive,
	.request_access = tpm2_fifo_request_access,
	.release_locality = tpm2_fifo_release_locality,
};

struct interface_ops *
tpm_interface_getops(struct tpm_chip_data *chip_data,  uint8_t locality)
{
	return &fifo_ops;
}
