/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SDMMC_H
#define SDMMC_H

#include <lib/mmio.h>
#include "socfpga_handoff.h"

#define PERIPHERAL_SDMMC_MASK			0x60
#define PERIPHERAL_SDMMC_OFFSET			6

#define DEFAULT_SDMMC_MAX_RETRIES		5
#define SEND_SDMMC_OP_COND_MAX_RETRIES		100
#define SDMMC_MULT_BY_512K_SHIFT		19

static const unsigned char tran_speed_base[16] = {
	0, 10, 12, 13, 15, 20, 26, 30, 35, 40, 45, 52, 55, 60, 70, 80
};

static const unsigned char sd_tran_speed_base[16] = {
	0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80
};


/* FUNCTION DEFINATION */
/*
 * @brief SDMMC controller initialization function
 *
 * @hoff_ptr: Pointer to the hand-off data
 * Return: 0 on success, a negative errno on failure
 */
void sdmmc_pin_config(void);
size_t sdmmc_read_blocks(int lba, uintptr_t buf, size_t size);
size_t sdmmc_write_blocks(int lba, const uintptr_t buf, size_t size);
int sdmmc_device_state(void);
bool is_cmd23_enabled(void);
int sdmmc_send_cmd(unsigned int idx, unsigned int arg,
			unsigned int r_type, unsigned int *r_data);

#endif
