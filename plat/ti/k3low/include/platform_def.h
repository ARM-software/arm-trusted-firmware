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

#define WKUP_CTRL_MMR_SEC_2_BASE	        (0x43020000UL)
#define WKUP_CTRL_MMR_SEC_4_BASE	        (0x43040000UL)
#define WKUP_CTRL_MMR_SEC_5_BASE	        (0x43050000UL)
#define K3_WKUP_UART_BASE_ADDRESS	        (0x2b300000U)
#define K3_MAIN_PSC_BASE		        (0x400000UL)
#define K3_MAIN_PLL_MMR_BASE		        (0x04060000UL)
#define K3_GTC_CFG0_BASE		        (0xa80000UL)
#define K3_GTC_CFG1_BASE		        (0xA90000UL)
#define K3_RTC_BASE			        (0x2b1f0000UL)

#define DEVICE_WKUP_SRAM_BASE			UL(0x707f0000)
#define DEVICE_WKUP_SRAM_STACK_SIZE		UL(0x1000)
#define DEVICE_WKUP_SRAM_SIZE			UL(0x00010000)
#define DEVICE_WKUP_SRAM_CODE_SIZE		(DEVICE_WKUP_SRAM_SIZE - \
						 DEVICE_WKUP_SRAM_STACK_SIZE)
#define DEVICE_WKUP_SRAM_STACK_BASE		(DEVICE_WKUP_SRAM_BASE + \
						 DEVICE_WKUP_SRAM_CODE_SIZE)
#define DEVICE_WKUP_SRAM_STACK_BASE_L	        (DEVICE_WKUP_SRAM_STACK_BASE & 0xFFFFU)
#define DEVICE_WKUP_SRAM_STACK_BASE_H	        (DEVICE_WKUP_SRAM_STACK_BASE >> 16)
#define __wkupsramfunc				__section(".wkupsram.text")
#define __wkupsramdata				__section(".wkupsram.data")
#define __wkupsramresumeentry			__section(".wkupsram.resume_entry")
#define __wkupsramsuspendentry			__section(".wkupsram.suspend_entry")

/*******************************************************************************
 * Memory layout constants
 ******************************************************************************/

/*
 * This RAM will be used for the bootloader including code, bss, and stacks.
 * It may need to be increased if BL31 grows in size.
 *
 * The link addresses are determined by BL31_BASE + offset.
 * When ENABLE_PIE is set, the TF images can be loaded anywhere, so
 * BL31_BASE is really arbitrary.
 *
 * When ENABLE_PIE is unset, BL31_BASE should be chosen so that
 * it matches to the physical address where BL31 is loaded, that is,
 * BL31_BASE should be the base address of the RAM region.
 *
 * Lets make things explicit by mapping BL31_BASE to 0x0 since ENABLE_PIE is
 * defined as default for our platform.
 */
#define BL31_BASE	UL(0x00000000) /* PIE remapped on fly */
#define BL31_SIZE	UL(0x00050000) /* 320k */
#define BL31_LIMIT	(BL31_BASE + BL31_SIZE)

#endif /* PLATFORM_DEF_H */
