/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HIKEY_DEF_H
#define HIKEY_DEF_H

/* Always assume DDR is 1GB size. */
#define DDR_BASE			0x0
#define DDR_SIZE			0x40000000

#define DEVICE_BASE			0xF4000000
#define DEVICE_SIZE			0x05800000

/* Memory location options for TSP */
#define HIKEY_SRAM_ID		0
#define HIKEY_DRAM_ID		1

/*
 * DDR for OP-TEE (32MB from 0x3E00000-0x3FFFFFFF) is divided in several
 * regions
 *   - Secure DDR (default is the top 16MB) used by OP-TEE
 *   - Non-secure DDR used by OP-TEE (shared memory and padding) (4MB)
 *   - Secure DDR (4MB aligned on 4MB) for OP-TEE's "Secure Data Path" feature
 *   - Non-secure DDR (8MB) reserved for OP-TEE's future use
 */
#define DDR_SEC_SIZE			0x01000000
#define DDR_SEC_BASE			(DDR_BASE + DDR_SIZE - DDR_SEC_SIZE) /* 0x3F000000 */

#define DDR_SDP_SIZE			0x00400000
#define DDR_SDP_BASE			(DDR_SEC_BASE - 0x400000 /* align */ - \
					DDR_SDP_SIZE)

#define SRAM_BASE			0xFFF80000
#define SRAM_SIZE			0x00012000

/*
 * PL011 related constants
 */
#define PL011_UART0_BASE		0xF8015000
#define PL011_UART2_BASE		0xF7112000
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

#define EMMC_BASE			0
#define HIKEY_FIP_BASE			(EMMC_BASE + (4 << 20))
#define HIKEY_FIP_MAX_SIZE		(8 << 20)
#define HIKEY_EMMC_RPMB_BASE		(EMMC_BASE + 0)
#define HIKEY_EMMC_RPMB_MAX_SIZE	(128 << 10)
#define HIKEY_EMMC_USERDATA_BASE	(EMMC_BASE + 0)
#define HIKEY_EMMC_USERDATA_MAX_SIZE	(4 << 30)

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

#endif /* HIKEY_DEF_H */
