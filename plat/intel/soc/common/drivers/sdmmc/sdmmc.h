/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
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
int sdmmc_init(handoff *hoff_ptr, struct cdns_sdmmc_params *params,
	     struct mmc_device_info *info);
int sd_or_mmc_init(const struct mmc_ops *ops_ptr, unsigned int clk,
	     unsigned int width, unsigned int flags,
	     struct mmc_device_info *device_info);
void sdmmc_pin_config(void);
#endif
