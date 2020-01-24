/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AXG_DEF_H
#define AXG_DEF_H

#include <lib/utils_def.h>

/*******************************************************************************
 * System oscillator
 ******************************************************************************/
#define AML_OSC24M_CLK_IN_HZ			ULL(24000000) /* 24 MHz */

/*******************************************************************************
 * Memory regions
 ******************************************************************************/
#define AML_NS_SHARE_MEM_BASE			UL(0x05000000)
#define AML_NS_SHARE_MEM_SIZE			UL(0x00100000)

#define AML_SEC_SHARE_MEM_BASE			UL(0x05200000)
#define AML_SEC_SHARE_MEM_SIZE			UL(0x00100000)

#define AML_GIC_DEVICE_BASE			UL(0xFFC00000)
#define AML_GIC_DEVICE_SIZE			UL(0x00008000)

#define AML_NSDRAM0_BASE			UL(0x01000000)
#define AML_NSDRAM0_SIZE			UL(0x0F000000)

#define BL31_BASE				UL(0x05100000)
#define BL31_SIZE				UL(0x00100000)
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

/* Shared memory used for SMC services */
#define AML_SHARE_MEM_INPUT_BASE		UL(0x050FE000)
#define AML_SHARE_MEM_OUTPUT_BASE		UL(0x050FF000)

#define AML_SEC_DEVICE0_BASE			UL(0xFFD00000)
#define AML_SEC_DEVICE0_SIZE			UL(0x00026000)

#define AML_SEC_DEVICE1_BASE			UL(0xFF800000)
#define AML_SEC_DEVICE1_SIZE			UL(0x0000A000)

#define AML_SEC_DEVICE2_BASE			UL(0xFF620000)
#define AML_SEC_DEVICE2_SIZE			UL(0x00028000)

#define AML_TZRAM_BASE				UL(0xFFFC0000)
#define AML_TZRAM_SIZE				UL(0x00020000)

/* Mailboxes */
#define AML_MHU_SECURE_SCP_TO_AP_PAYLOAD	UL(0xFFFD3800)
#define AML_MHU_SECURE_AP_TO_SCP_PAYLOAD	UL(0xFFFD3A00)
#define AML_PSCI_MAILBOX_BASE			UL(0xFFFD3F00)

/*******************************************************************************
 * GIC-400 and interrupt handling related constants
 ******************************************************************************/
#define AML_GICD_BASE				UL(0xFFC01000)
#define AML_GICC_BASE				UL(0xFFC02000)

#define IRQ_SEC_PHY_TIMER			29

#define IRQ_SEC_SGI_0				8
#define IRQ_SEC_SGI_1				9
#define IRQ_SEC_SGI_2				10
#define IRQ_SEC_SGI_3				11
#define IRQ_SEC_SGI_4				12
#define IRQ_SEC_SGI_5				13
#define IRQ_SEC_SGI_6				14
#define IRQ_SEC_SGI_7				15
#define IRQ_SEC_SGI_8				16

/*******************************************************************************
 * UART definitions
 ******************************************************************************/
#define AML_UART0_AO_BASE			UL(0xFF803000)
#define AML_UART0_AO_CLK_IN_HZ			AML_OSC24M_CLK_IN_HZ
#define AML_UART_BAUDRATE			U(115200)

/*******************************************************************************
 * Memory-mapped I/O Registers
 ******************************************************************************/
#define AML_AO_TIMESTAMP_CNTL			UL(0xFF8000B4)

#define AML_SYS_CPU_CFG7			UL(0xFF634664)

#define AML_AO_RTI_STATUS_REG3			UL(0xFF80001C)
#define AML_AO_RTI_SCP_STAT			UL(0xFF80023C)
#define AML_AO_RTI_SCP_READY_OFF		U(0x14)
#define AML_A0_RTI_SCP_READY_MASK		U(3)
#define AML_AO_RTI_SCP_IS_READY(v)					\
	((((v) >> AML_AO_RTI_SCP_READY_OFF) &				\
	AML_A0_RTI_SCP_READY_MASK) == AML_A0_RTI_SCP_READY_MASK)

#define AML_HIU_MAILBOX_SET_0			UL(0xFF63C404)
#define AML_HIU_MAILBOX_STAT_0			UL(0xFF63C408)
#define AML_HIU_MAILBOX_CLR_0			UL(0xFF63C40C)
#define AML_HIU_MAILBOX_SET_3			UL(0xFF63C428)
#define AML_HIU_MAILBOX_STAT_3			UL(0xFF63C42C)
#define AML_HIU_MAILBOX_CLR_3			UL(0xFF63C430)

#define AML_SHA_DMA_BASE			UL(0xFF63E000)
#define AML_SHA_DMA_DESC			(AML_SHA_DMA_BASE + 0x08)
#define AML_SHA_DMA_STATUS			(AML_SHA_DMA_BASE + 0x28)

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

#endif /* AXG_DEF_H */
