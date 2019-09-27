/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GXBB_DEF_H
#define GXBB_DEF_H

#include <lib/utils_def.h>

/*******************************************************************************
 * System oscillator
 ******************************************************************************/
#define AML_OSC24M_CLK_IN_HZ			ULL(24000000) /* 24 MHz */

/*******************************************************************************
 * Memory regions
 ******************************************************************************/
#define AML_NSDRAM0_BASE			UL(0x01000000)
#define AML_NSDRAM0_SIZE			UL(0x0F000000)

#define AML_NSDRAM1_BASE			UL(0x10000000)
#define AML_NSDRAM1_SIZE			UL(0x00100000)

#define BL31_BASE				UL(0x10100000)
#define BL31_SIZE				UL(0x000C0000)
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

/* Shared memory used for SMC services */
#define AML_SHARE_MEM_INPUT_BASE		UL(0x100FE000)
#define AML_SHARE_MEM_OUTPUT_BASE		UL(0x100FF000)

#define AML_SEC_DEVICE0_BASE			UL(0xC0000000)
#define AML_SEC_DEVICE0_SIZE			UL(0x09000000)

#define AML_SEC_DEVICE1_BASE			UL(0xD0040000)
#define AML_SEC_DEVICE1_SIZE			UL(0x00008000)

#define AML_TZRAM_BASE				UL(0xD9000000)
#define AML_TZRAM_SIZE				UL(0x00014000)
/* Top 0xC000 bytes (up to 0xD9020000) used by BL2 */

/* Mailboxes */
#define AML_MHU_SECURE_SCP_TO_AP_PAYLOAD	UL(0xD9013800)
#define AML_MHU_SECURE_AP_TO_SCP_PAYLOAD	UL(0xD9013A00)
#define AML_PSCI_MAILBOX_BASE			UL(0xD9013F00)

#define AML_TZROM_BASE				UL(0xD9040000)
#define AML_TZROM_SIZE				UL(0x00010000)

#define AML_SEC_DEVICE2_BASE			UL(0xDA000000)
#define AML_SEC_DEVICE2_SIZE			UL(0x00200000)

#define AML_SEC_DEVICE3_BASE			UL(0xDA800000)
#define AML_SEC_DEVICE3_SIZE			UL(0x00200000)

/*******************************************************************************
 * GIC-400 and interrupt handling related constants
 ******************************************************************************/
#define AML_GICD_BASE				UL(0xC4301000)
#define AML_GICC_BASE				UL(0xC4302000)

#define IRQ_SEC_PHY_TIMER			29

#define IRQ_SEC_SGI_0				8
#define IRQ_SEC_SGI_1				9
#define IRQ_SEC_SGI_2				10
#define IRQ_SEC_SGI_3				11
#define IRQ_SEC_SGI_4				12
#define IRQ_SEC_SGI_5				13
#define IRQ_SEC_SGI_6				14
#define IRQ_SEC_SGI_7				15

/*******************************************************************************
 * UART definitions
 ******************************************************************************/
#define AML_UART0_AO_BASE			UL(0xC81004C0)
#define AML_UART0_AO_CLK_IN_HZ			AML_OSC24M_CLK_IN_HZ
#define AML_UART_BAUDRATE			U(115200)

/*******************************************************************************
 * Memory-mapped I/O Registers
 ******************************************************************************/
#define AML_AO_TIMESTAMP_CNTL			UL(0xC81000B4)

#define AML_SYS_CPU_CFG7			UL(0xC8834664)

#define AML_AO_RTI_STATUS_REG3			UL(0xDA10001C)

#define AML_HIU_MAILBOX_SET_0			UL(0xDA83C404)
#define AML_HIU_MAILBOX_STAT_0			UL(0xDA83C408)
#define AML_HIU_MAILBOX_CLR_0			UL(0xDA83C40C)
#define AML_HIU_MAILBOX_SET_3			UL(0xDA83C428)
#define AML_HIU_MAILBOX_STAT_3			UL(0xDA83C42C)
#define AML_HIU_MAILBOX_CLR_3			UL(0xDA83C430)

#define AML_SHA_DMA_BASE			UL(0xC883E000)
#define AML_SHA_DMA_DESC			(AML_SHA_DMA_BASE + 0x08)
#define AML_SHA_DMA_STATUS			(AML_SHA_DMA_BASE + 0x18)

/*******************************************************************************
 * System Monitor Call IDs and arguments
 ******************************************************************************/
#define AML_SM_GET_SHARE_MEM_INPUT_BASE		U(0x82000020)
#define AML_SM_GET_SHARE_MEM_OUTPUT_BASE	U(0x82000021)

#define AML_SM_EFUSE_READ			U(0x82000030)
#define AML_SM_EFUSE_USER_MAX			U(0x82000033)

#define AML_SM_JTAG_ON				U(0x82000040)
#define AML_SM_JTAG_OFF				U(0x82000041)
#define AML_SM_GET_CHIP_ID			U(0x82000044)

#define AML_JTAG_STATE_ON			U(0)
#define AML_JTAG_STATE_OFF			U(1)

#define AML_JTAG_M3_AO				U(0)
#define AML_JTAG_M3_EE				U(1)
#define AML_JTAG_A53_AO				U(2)
#define AML_JTAG_A53_EE				U(3)

#endif /* GXBB_DEF_H */
