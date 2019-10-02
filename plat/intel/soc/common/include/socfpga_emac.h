/*
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_EMAC_H
#define SOCFPGA_EMAC_H

/* EMAC PHY Mode */

#define PHY_INTERFACE_MODE_GMII_MII		0
#define PHY_INTERFACE_MODE_RGMII		1
#define PHY_INTERFACE_MODE_RMII			2
#define PHY_INTERFACE_MODE_RESET		3

/* Mask Definitions */

#define PHY_INTF_SEL_MSK			0x3
#define FPGAINTF_EN_3_EMAC_MSK(x)		(1 << (x * 8))

void socfpga_emac_init(void);

#endif /* SOCFPGA_EMAC_H */
