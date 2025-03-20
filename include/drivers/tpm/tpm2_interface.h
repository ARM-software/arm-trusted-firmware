/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TPM2_INTERFACE_H
#define TPM2_INTERFACE_H

#include "tpm2_chip.h"

typedef struct interface_ops {
	int (*get_info)(struct tpm_chip_data *chip_data, uint8_t locality);
	int (*send)(struct tpm_chip_data *chip_data, const tpm_cmd *buf);
	int (*receive)(struct tpm_chip_data *chip_data, tpm_cmd *buf);
	int (*request_access)(struct tpm_chip_data *chip_data, uint8_t locality);
	int (*release_locality)(struct tpm_chip_data *chip_data, uint8_t locality);
} interface_ops_t;

struct interface_ops *tpm_interface_getops(struct tpm_chip_data *chip_data, uint8_t locality);

int tpm2_fifo_write_byte(uint16_t tpm_reg, uint8_t val);

int tpm2_fifo_read_byte(uint16_t tpm_reg, uint8_t *val);

int tpm2_fifo_read_chunk(uint16_t tpm_reg, uint8_t len, void *val);

#endif /* TPM2_INTERFACE_H */
