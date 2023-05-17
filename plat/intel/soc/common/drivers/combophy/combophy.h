/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMBOPHY_H
#define COMBOPHY_H

#include <lib/mmio.h>

#include "socfpga_handoff.h"

#define PERIPHERAL_SDMMC_MASK		0x60
#define PERIPHERAL_SDMMC_OFFSET		6
#define DFI_INTF_MASK			0x1

/* FUNCTION DEFINATION */
/*
 * @brief Nand controller initialization function
 *
 * @hoff_ptr: Pointer to the hand-off data
 * Return: 0 on success, a negative errno on failure
 */
int combo_phy_init(handoff *hoff_ptr);
int dfi_select(handoff *hoff_ptr);

#endif
