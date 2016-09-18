/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __HIKEY_DEF_H__
#define __HIKEY_DEF_H__

#include <common_def.h>
#include <tbbr_img_def.h>

/* Always assume DDR is 1GB size. */
#define DDR_BASE			0x0
#define DDR_SIZE			0x80000000

#define DEVICE_BASE			0xF4000000
#define DEVICE_SIZE			0x05800000

#define XG2RAM0_BASE			0xF9800000
#define XG2RAM0_SIZE			0x00400000

#define SRAM_BASE			0xFFF80000
#define SRAM_SIZE			0x00012000

/*
 * BL1 is stored in XG2RAM0_HIRQ that is 784KB large (0xF980_0000~0xF98C_4000).
 */
#define ONCHIPROM_PARAM_BASE		(XG2RAM0_BASE + 0x700)
#define LOADER_RAM_BASE			(XG2RAM0_BASE + 0x800)
#define BL1_XG2RAM0_OFFSET		0x1000

/*
 * PL011 related constants
 */
#define PL011_UART0_BASE		0xF8015000
#define PL011_UART3_BASE		0xF7113000
#define PL011_BAUDRATE			115200
#define PL011_UART_CLK_IN_HZ		19200000

#define HIKEY_USB_DESC_BASE		(DDR_BASE + 0x00800000)
#define HIKEY_USB_DESC_SIZE		0x00100000
#define HIKEY_USB_DATA_BASE		(DDR_BASE + 0x10000000)
#define HIKEY_USB_DATA_SIZE		0x10000000
#define HIKEY_FB_BUFFER_BASE		(HIKEY_USB_DATA_BASE)
#define HIKEY_FB_BUFFER_SIZE		HIKEY_USB_DATA_SIZE
#define HIKEY_FB_DOWNLOAD_BASE		(HIKEY_FB_BUFFER_BASE +		\
					 HIKEY_FB_BUFFER_SIZE)
#define HIKEY_FB_DOWNLOAD_SIZE		HIKEY_USB_DATA_SIZE

#define HIKEY_USB_DESC_IN_BASE		(DDR_BASE + 0x00800000)
#define HIKEY_USB_DESC_IN_SIZE		0x00040000
#define HIKEY_USB_DESC_EP0_OUT_BASE	(HIKEY_USB_DESC_IN_BASE +	\
					 HIKEY_USB_DESC_IN_SIZE)
#define HIKEY_USB_DESC_EP0_OUT_SIZE	0x00040000
#define HIKEY_USB_DESC_EPX_OUT_BASE	(HIKEY_USB_DESC_EP0_OUT_BASE +	\
					 HIKEY_USB_DESC_EP0_OUT_SIZE)
#define HIKEY_USB_DESC_EPX_OUT_SIZE	0x00080000

#define HIKEY_MMC_DESC_BASE		(DDR_BASE + 0x03000000)
#define HIKEY_MMC_DESC_SIZE		0x00100000

/*
 * HIKEY_MMC_DATA_BASE & HIKEY_MMC_DATA_SIZE are shared between fastboot
 * and eMMC driver. Since it could avoid to memory copy.
 * So this SRAM region is used twice. First, it's used in BL1 as temporary
 * buffer in eMMC driver. Second, it's used by MCU in BL2. The SRAM region
 * needs to be clear before used in BL2.
 */
#define HIKEY_MMC_DATA_BASE		(DDR_BASE + 0x10000000)
#define HIKEY_MMC_DATA_SIZE		0x20000000
#define HIKEY_NS_IMAGE_OFFSET		(DDR_BASE + 0x35000000)
#define HIKEY_BL1_MMC_DESC_BASE		(SRAM_BASE)
#define HIKEY_BL1_MMC_DESC_SIZE		0x00001000
#define HIKEY_BL1_MMC_DATA_BASE		(HIKEY_BL1_MMC_DESC_BASE +	\
					 HIKEY_BL1_MMC_DESC_SIZE)
#define HIKEY_BL1_MMC_DATA_SIZE		0x0000B000

#define MMC_BASE			0
#define HIKEY_FIP_BASE			(MMC_BASE + (4 << 20))
#define HIKEY_FIP_MAX_SIZE		(8 << 20)
#define HIKEY_NSBL1U_BASE		(MMC_BASE + 0)
#define HIKEY_NSBL1U_MAX_SIZE		(128 << 10)
#define HIKEY_BL2U_BASE			(MMC_BASE + 0)
#define HIKEY_BL2U_MAX_SIZE		(4 << 30)

/*
 * GIC400 interrupt handling related constants
 */
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

#endif /* __HIKEY_DEF_H__ */
