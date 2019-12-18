/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NCSI_H
#define NCSI_H

/*
 * There are 10 registers for NCSI IO drivers.
 */
#define NITRO_NCSI_IOPAD_CONTROL_NUM	10
#define NITRO_NCSI_IOPAD_CONTROL_BASE	0x60e05080

/*
 * NCSI IO Drive strength
 *   000 - Drives 2mA
 *   001 - Drives 4mA
 *   010 - Drives 6mA
 *   011 - Drives 8mA
 *   100 - Drives 10mA
 *   101 - Drives 12mA
 *   110 - Drives 14mA
 *   111 - Drives 16mA
 */
#define PAD_SELX_VALUE(selx)	((selx) << 1)
#define PAD_SELX_MASK		(0x7 << 1)

void brcm_stingray_ncsi_init(void);

#endif
