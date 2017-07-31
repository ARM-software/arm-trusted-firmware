/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_CSS_DEF_H__
#define __SOC_CSS_DEF_H__

#include <common_def.h>
#include <utils_def.h>


/*
 * Definitions common to all ARM CSS SoCs
 */

/* Following covers ARM CSS SoC Peripherals and PCIe expansion area */
#define SOC_CSS_DEVICE_BASE		0x40000000
#define SOC_CSS_DEVICE_SIZE		0x40000000
#define SOC_CSS_PCIE_CONTROL_BASE	0x7ff20000

/* PL011 UART related constants */
#define SOC_CSS_UART0_BASE		0x7ff80000
#define SOC_CSS_UART1_BASE		0x7ff70000

#define SOC_CSS_UART0_CLK_IN_HZ		7372800
#define SOC_CSS_UART1_CLK_IN_HZ		7372800

/* SoC NIC-400 Global Programmers View (GPV) */
#define SOC_CSS_NIC400_BASE		0x7fd00000

#define SOC_CSS_NIC400_USB_EHCI		0
#define SOC_CSS_NIC400_TLX_MASTER	1
#define SOC_CSS_NIC400_USB_OHCI		2
#define SOC_CSS_NIC400_PL354_SMC	3
/*
 * The apb4_bridge controls access to:
 *   - the PCIe configuration registers
 *   - the MMU units for USB, HDLCD and DMA
 */
#define SOC_CSS_NIC400_APB4_BRIDGE	4

/* Non-volatile counters */
#define SOC_TRUSTED_NVCTR_BASE		0x7fe70000
#define TFW_NVCTR_BASE			(SOC_TRUSTED_NVCTR_BASE + 0x0000)
#define TFW_NVCTR_SIZE			4
#define NTFW_CTR_BASE			(SOC_TRUSTED_NVCTR_BASE + 0x0004)
#define NTFW_CTR_SIZE			4

/* Keys */
#define SOC_KEYS_BASE			0x7fe80000
#define TZ_PUB_KEY_HASH_BASE		(SOC_KEYS_BASE + 0x0000)
#define TZ_PUB_KEY_HASH_SIZE		32
#define HU_KEY_BASE			(SOC_KEYS_BASE + 0x0020)
#define HU_KEY_SIZE			16
#define END_KEY_BASE			(SOC_KEYS_BASE + 0x0044)
#define END_KEY_SIZE			32

#define SOC_CSS_MAP_DEVICE		MAP_REGION_FLAT(		\
						SOC_CSS_DEVICE_BASE,	\
						SOC_CSS_DEVICE_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)


/*
 * The bootsec_bridge controls access to a bunch of peripherals, e.g. the UARTs.
 */
#define SOC_CSS_NIC400_BOOTSEC_BRIDGE	5
#define SOC_CSS_NIC400_BOOTSEC_BRIDGE_UART1	(1 << 12)

/*
 * Required platform porting definitions common to all ARM CSS SoCs
 */
#if JUNO_AARCH32_EL3_RUNTIME
/*
 * Following change is required to initialize TZC
 * for enabling access to the HI_VECTOR (0xFFFF0000)
 * location needed for JUNO AARCH32 support.
 */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	ULL(0x8000)
#else
/* 2MB used for SCP DDR retraining */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	ULL(0x00200000)
#endif

#endif /* __SOC_CSS_DEF_H__ */
