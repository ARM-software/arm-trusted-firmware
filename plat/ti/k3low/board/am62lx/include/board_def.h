/*
 * Copyright (C) 2024-2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_DEF_H
#define BOARD_DEF_H

#include <lib/utils_def.h>

/* The ports must be in order and contiguous */
#define K3_CLUSTER0_CORE_COUNT		U(2)
#define K3_CLUSTER1_CORE_COUNT		U(0)
#define K3_CLUSTER2_CORE_COUNT		U(0)
#define K3_CLUSTER3_CORE_COUNT		U(0)

#define PLAT_PROC_START_ID		U(32)
#define PLAT_PROC_DEVICE_START_ID	U(135)
#define PLAT_CLUSTER_DEVICE_START_ID	U(134)
#define PLAT_BOARD_DEVICE_ID		U(157)

/* Pre-decided SRAM Addresses for sending and receiving messages */
#define MAILBOX_TX_START_REGION		UL(0x70814000)
#define MAILBOX_RX_START_REGION		UL(0x70815000)
/*
 * Pre-calculated MAX size of a message
 * sec_hdr + (type/host/seq + flags) + payload
 * 4 + 16 + 36
 */
#define MAILBOX_MAX_MESSAGE_SIZE	U(56)

#endif /* BOARD_DEF_H */
