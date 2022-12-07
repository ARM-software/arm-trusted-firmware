/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PSA_MEASURED_BOOT_PRIVATE_H
#define PSA_MEASURED_BOOT_PRIVATE_H

#include <stdint.h>

/* Measured boot message types that distinguish its services */
#define RSS_MEASURED_BOOT_READ		1001U
#define RSS_MEASURED_BOOT_EXTEND	1002U

struct measured_boot_read_iovec_in_t {
    uint8_t index;
    uint8_t sw_type_size;
    uint8_t version_size;
};

struct measured_boot_read_iovec_out_t {
    uint8_t  is_locked;
    uint32_t measurement_algo;
    uint8_t  sw_type[SW_TYPE_MAX_SIZE];
    uint8_t  sw_type_len;
    uint8_t  version[VERSION_MAX_SIZE];
    uint8_t  version_len;
};

struct measured_boot_extend_iovec_t {
	uint8_t  index;
	uint8_t  lock_measurement;
	uint32_t measurement_algo;
	uint8_t  sw_type[SW_TYPE_MAX_SIZE];
	uint8_t  sw_type_size;
};

#endif /* PSA_MEASURED_BOOT_PRIVATE_H */
