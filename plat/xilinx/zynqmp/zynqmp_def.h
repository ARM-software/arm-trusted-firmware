/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __ZYNQMP_DEF_H__
#define __ZYNQMP_DEF_H__

#include <common_def.h>

/* Firmware Image Package */
#define ZYNQMP_PRIMARY_CPU		0

/* Memory location options for Shared data and TSP in ZYNQMP */
#define ZYNQMP_IN_TRUSTED_SRAM		0
#define ZYNQMP_IN_TRUSTED_DRAM		1

/*******************************************************************************
 * ZYNQMP memory map related constants
 ******************************************************************************/

#define ZYNQMP_TRUSTED_SRAM_BASE	0xFFFC0000
#define ZYNQMP_TRUSTED_SRAM_SIZE	0x00040000
#define ZYNQMP_TRUSTED_SRAM_LIMIT	(ZYNQMP_TRUSTED_SRAM_BASE + \
					 ZYNQMP_TRUSTED_SRAM_SIZE)


/* Location of trusted dram on the base zynqmp */
#define ZYNQMP_TRUSTED_DRAM_BASE	0x30000000 /* Can't overlap TZROM area */
#define ZYNQMP_TRUSTED_DRAM_SIZE	0x10000000
#define ZYNQMP_TRUSTED_DRAM_LIMIT	(ZYNQMP_TRUSTED_DRAM_BASE + \
					 ZYNQMP_TRUSTED_DRAM_SIZE)

/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE		0xFF000000
#define DEVICE0_SIZE		0x00E00000
#define DEVICE1_BASE		0xF9000000
#define DEVICE1_SIZE		0x01000000

/* For cpu reset APU space here too 0xFE5F1000 CRF_APB*/
#define CRF_APB_BASE		0xFD1A0000
#define CRF_APB_SIZE		0x00600000

/* CRF registers and bitfields */
#define CRF_APB_RST_FPD_APU	(CRF_APB_BASE + 0X00000104)

#define CRF_APB_RST_FPD_APU_ACPU_RESET		(1 << 0)
#define CRF_APB_RST_FPD_APU_ACPU_PWRON_RESET	(1 << 10)

/* CRL registers and bitfields */
#define CRL_APB_BASE			0xFF5E0000
#define CRL_APB_RPLL_CTRL		(CRL_APB_BASE + 0x30)
#define CRL_APB_TIMESTAMP_REF_CTRL	(CRL_APB_BASE + 0x128)
#define CRL_APB_RESET_CTRL		(CRL_APB_BASE + 0x218)

#define CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT	(1 << 24)

#define CRL_APB_RPLL_CTRL_BYPASS	(1 << 3)

#define CRL_APB_RESET_CTRL_SOFT_RESET	(1 << 4)

/* system counter registers and bitfields */
#define IOU_SCNTRS_BASE			0xFF260000
#define IOU_SCNTRS_CONTROL		(IOU_SCNTRS_BASE + 0)
#define IOU_SCNTRS_BASEFREQ		(IOU_SCNTRS_BASE + 0x20)

#define IOU_SCNTRS_CONTROL_EN		(1 << 0)

/* APU registers and bitfields */
#define APU_BASE		0xFD5C0000
#define APU_CONFIG_0		(APU_BASE + 0x20)
#define APU_RVBAR_L_0		(APU_BASE + 0x40)
#define APU_RVBAR_H_0		(APU_BASE + 0x44)
#define APU_PWRCTL		(APU_BASE + 0x90)

#define APU_CONFIG_0_VINITHI_SHIFT	8
#define APU_0_PWRCTL_CPUPWRDWNREQ_MASK		1
#define APU_1_PWRCTL_CPUPWRDWNREQ_MASK		2
#define APU_2_PWRCTL_CPUPWRDWNREQ_MASK		4
#define APU_3_PWRCTL_CPUPWRDWNREQ_MASK		8

/* PMU registers and bitfields */
#define PMU_GLOBAL_BASE			0xFFD80000
#define PMU_GLOBAL_CNTRL		(PMU_GLOBAL_BASE + 0)
#define PMU_GLOBAL_REQ_PWRUP_STATUS	(PMU_GLOBAL_BASE + 0x110)
#define PMU_GLOBAL_REQ_PWRUP_EN		(PMU_GLOBAL_BASE + 0x118)
#define PMU_GLOBAL_REQ_PWRUP_DIS	(PMU_GLOBAL_BASE + 0x11c)
#define PMU_GLOBAL_REQ_PWRUP_TRIG	(PMU_GLOBAL_BASE + 0x120)

#define PMU_GLOBAL_CNTRL_FW_IS_PRESENT	(1 << 4)

#define DRAM1_BASE		0x00000000ull
#define DRAM1_SIZE		0x10000000ull
#define DRAM1_END		(DRAM1_BASE + DRAM1_SIZE - 1)

#define DRAM_BASE		DRAM1_BASE
#define DRAM_SIZE		DRAM1_SIZE

/* Load address of BL33 in the ZYNQMP port */
#define PLAT_ARM_NS_IMAGE_OFFSET	(DRAM1_BASE + 0x8000000) /* DRAM + 128MB */

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_ARM_CCI_BASE		0xFD6E0000
#define PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX	3
#define PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX	4

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
#define BASE_GICD_BASE		0xF9010000
#define BASE_GICC_BASE		0xF9020000
#define BASE_GICH_BASE		0xF9040000
#define BASE_GICV_BASE		0xF9060000

#define IRQ_SEC_IPI_APU			67
#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

#define MAX_INTR_EL3			128

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define ZYNQMP_UART0_BASE		0xFF000000
#define ZYNQMP_UART1_BASE		0xFF001000

#define PLAT_ARM_CRASH_UART_BASE	ZYNQMP_UART0_BASE
/* impossible to call C routine how it is done now - hardcode any value */
#define	PLAT_ARM_CRASH_UART_CLK_IN_HZ	100000000 /* FIXME */

/* Must be non zero */
#define ZYNQMP_UART_BAUDRATE	115200
#define ARM_CONSOLE_BAUDRATE	ZYNQMP_UART_BAUDRATE

/* Silicon version detection */
#define ZYNQMP_SILICON_VER_MASK		0xF000
#define ZYNQMP_SILICON_VER_SHIFT	12
#define ZYNQMP_CSU_VERSION_SILICON	0
#define ZYNQMP_CSU_VERSION_EP108	1
#define ZYNQMP_CSU_VERSION_VELOCE	2
#define ZYNQMP_CSU_VERSION_QEMU		3

#define ZYNQMP_RTL_VER_MASK		0xFF0
#define ZYNQMP_RTL_VER_SHIFT		4

#define ZYNQMP_PS_VER_MASK		0xF
#define ZYNQMP_PS_VER_SHIFT		0

#define ZYNQMP_CSU_BASEADDR		0xFFCA0000
#define ZYNQMP_CSU_IDCODE_OFFSET	0x40

#define ZYNQMP_CSU_IDCODE_XILINX_ID_SHIFT	0
#define ZYNQMP_CSU_IDCODE_XILINX_ID_MASK	(0xFFF << ZYNQMP_CSU_IDCODE_XILINX_ID_SHIFT)
#define ZYNQMP_CSU_IDCODE_XILINX_ID		0x093

#define ZYNQMP_CSU_IDCODE_SVD_SHIFT		12
#define ZYNQMP_CSU_IDCODE_SVD_MASK		(0xE << ZYNQMP_CSU_IDCODE_SVD_SHIFT)
#define ZYNQMP_CSU_IDCODE_DEVICE_CODE_SHIFT	15
#define ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK	(0xF << ZYNQMP_CSU_IDCODE_DEVICE_CODE_SHIFT)
#define ZYNQMP_CSU_IDCODE_SUB_FAMILY_SHIFT	19
#define ZYNQMP_CSU_IDCODE_SUB_FAMILY_MASK	(0x3 << ZYNQMP_CSU_IDCODE_SUB_FAMILY_SHIFT)
#define ZYNQMP_CSU_IDCODE_FAMILY_SHIFT		21
#define ZYNQMP_CSU_IDCODE_FAMILY_MASK		(0x7F << ZYNQMP_CSU_IDCODE_FAMILY_SHIFT)
#define ZYNQMP_CSU_IDCODE_FAMILY		0x23

#define ZYNQMP_CSU_IDCODE_REVISION_SHIFT	28
#define ZYNQMP_CSU_IDCODE_REVISION_MASK		(0xF << ZYNQMP_CSU_IDCODE_REVISION_SHIFT)
#define ZYNQMP_CSU_IDCODE_REVISION		0

#define ZYNQMP_CSU_VERSION_OFFSET	0x44

#endif /* __ZYNQMP_DEF_H__ */
