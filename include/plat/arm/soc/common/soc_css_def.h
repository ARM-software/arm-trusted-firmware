/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SOC_CSS_DEF_H__
#define __SOC_CSS_DEF_H__

#include <common_def.h>
#include <xlat_tables.h>


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

#define SOC_CSS_UART0_CLK_IN_HZ		7273800
#define SOC_CSS_UART1_CLK_IN_HZ		7273800

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

/* 2MB used for SCP DDR retraining */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	MAKE_ULL(0x00200000)


#endif /* __SOC_CSS_DEF_H__ */
