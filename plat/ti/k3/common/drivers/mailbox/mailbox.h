/*
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ti_sci_transport.h>

#define TIFS_MAILBOX_BASE0		UL(0x44240000) /* TFA sending IPC messages to TIFS */
#define TIFS_MAILBOX_BASE1		UL(0x44250000) /* TIFS sending IPC messages to A53 */
#define TIFS_MAILBOX_SYSC		UL(0x10)

#define TIFS_MAILBOX_MSG		UL(0x40)
#define TIFS_MAILBOX_MSG_STATUS		UL(0xc0)

#define AM62L_RSVD_SRAM_BASE		UL(0x70814000)
#define AM62L_SRAM_SIZE			UL(0x2000)

