/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2025, Altera Corporation. All rights reserved.
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

/* Peripheral configuration, power gate enable bit masks */
#define POWER_GATE_EN_USB2_X		BIT(0)
#define POWER_GATE_EN_USB3_X		BIT(1)
#define POWER_GATE_EN_EMAC0		BIT(2)
#define POWER_GATE_EN_EMAC1		BIT(3)
#define POWER_GATE_EN_EMAC2		BIT(4)
#define POWER_GATE_EN_NAND		BIT(5)
#define POWER_GATE_EN_SDEMMC		BIT(6)
#define POWER_GATE_EN_CORESIGHT		BIT(7)

#define DFI_CTRL_SEL_HPNFC		0x00	/* Non-volatile flash controller */
#define DFI_CTRL_SEL_SDEMMC		0x01	/* SDMMC controller */

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