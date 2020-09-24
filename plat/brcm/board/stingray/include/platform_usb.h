/*
 * Copyright (c) 2019 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_USB_H
#define PLATFORM_USB_H

#include <platform_def.h>

#define USB3_DRD		BIT(0U)
#define USB3H_USB2DRD		BIT(1U)

extern const unsigned int xhc_portsc_reg_offset[MAX_USB_PORTS];

void xhci_phy_init(void);

#endif /* PLATFORM_USB_H */
