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

#define MAILBOX_TX_START_REGION		UL(0x70814000)
#define MAILBOX_TX_REGION_SIZE		UL(0x7081413F)

#define TIFS_MESSAGE_RESP_START_REGION		(0x70814000)
#define TIFS_MESSAGE_RESP_END_REGION		(0x7081513F)

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
#define DEVICE_WKUP_SRAM_CODE_SIZE		(DEVICE_WKUP_SRAM_SIZE - DEVICE_WKUP_SRAM_STACK_SIZE)
#define DEVICE_WKUP_SRAM_STACK_BASE		(DEVICE_WKUP_SRAM_BASE + DEVICE_WKUP_SRAM_CODE_SIZE)
#define DEVICE_WKUP_SRAM_STACK_BASE_L	        (DEVICE_WKUP_SRAM_STACK_BASE & 0xFFFFU)
#define DEVICE_WKUP_SRAM_STACK_BASE_H	        (DEVICE_WKUP_SRAM_STACK_BASE >> 16)
#define __wkupsramfunc				__section(".wkupsram.text")
#define __wkupsramdata				__section(".wkupsram.data")
#define __wkupsramresumeentry			__section(".wkupsram.resume_entry")
#define __wkupsramsuspendentry			__section(".wkupsram.suspend_entry")

#endif /* BOARD_DEF_H */
