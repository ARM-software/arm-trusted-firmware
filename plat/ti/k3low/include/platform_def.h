/*
 * Copyright (c) 2025, Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <ti_platform_defs.h>

#define TI_MAILBOX_TX_BASE		UL(0x44240000) /* TFA sending IPC messages to TIFS */
#define TI_MAILBOX_RX_BASE		UL(0x44250000) /* TIFS sending IPC messages to A53 */

/* We just need 512 bytes starting from TX/RX each, but simpler to just remap a 128K page */
#define TI_MAILBOX_RX_TX_SIZE		UL(0x20000)

#define WKUP_CTRL_MMR0_BASE		UL(0x43000000)
#define WKUP_CTRL_MMR0_SIZE		UL(0x80000)

#define WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET	UL(0x54000)

#endif /* PLATFORM_DEF_H */
