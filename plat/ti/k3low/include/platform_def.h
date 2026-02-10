/*
 * Copyright (c) 2026, Texas Instruments Incorporated - https://www.ti.com/
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

#define EMIF_CTLCFG_BASE UL(0x0F308000)
#define EMIF_CTLCFG_SIZE UL(0x8000)

#define WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET	UL(0x54000)

/* Device Control Region - covers PSC, PLL, and other control registers */
#define K3LOW_DEVCTRL_BASE    UL(0x200000)    /* Start of device control registers */
#define K3LOW_DEVCTRL_SIZE    UL(0x9380000)   /* Size of device control region */

#define TI_SCMI_CHANNELS		U(0x1)

#define BL1_RO_BASE	0x70800000 /* SRAM base address used as readonly segment */
#define BL1_RO_LIMIT	0x7080BFFF /* end of readonly segment */

#define	BL1_RW_BASE	0x7080C000 /* RW segment for data, stack and others */
#define	BL1_RW_LIMIT	0x70810000 /* end of RW segment */

#define BL2_BASE	0x80000000 /* BL2 base at start of DDR memory */
#define BL2_LIMIT	0x100000000 /* BL2 limit */

#define MAX_IO_HANDLES 1
#define MAX_IO_DEVICES 1

#define PLAT_PROC_START_ID              U(32)
#define PLAT_PROC_DEVICE_START_ID       U(135)
#define PLAT_CLUSTER_DEVICE_START_ID    U(134)
#define PLAT_BOARD_DEVICE_ID            U(157)

#define MAILBOX_SHMEM_REGION_BASE       UL(0x70810000)
#define MAILBOX_SHMEM_REGION_SIZE       UL(0x6000)

/* Pre-decided SRAM Addresses for sending and receiving messages */
#define MAILBOX_TX_START_REGION         UL(0x70814000)
#define MAILBOX_RX_START_REGION         UL(0x70815000)
/* 1 slot in the memory buffer dedicated for IPC is 64 bytes */
#define MAILBOX_RX_SLOT_SZ              U(64)
/* There are 5 slots in the memory buffer dedicated for IPC */
#define MAILBOX_RX_NUM_SLOTS            U(5)
/*
 * Pre-calculated MAX size of a message
 * sec_hdr + (type/host/seq + flags) + payload
 * 4 + 16 + 36
 */
#define MAILBOX_MAX_MESSAGE_SIZE        U(56)

/* Ensure the RX Slot size is not smaller than the max message size */
#if (MAILBOX_MAX_MESSAGE_SIZE > MAILBOX_RX_SLOT_SZ)
#error "MAILBOX_MAX_MESSAGE_SIZE > MAILBOX_RX_SLOT_SZ"
#endif

#endif /* PLATFORM_DEF_H */
