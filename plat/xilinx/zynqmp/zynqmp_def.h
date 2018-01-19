/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ZYNQMP_DEF_H__
#define __ZYNQMP_DEF_H__

#include <common_def.h>

#define ZYNQMP_CONSOLE_ID_cadence	1
#define ZYNQMP_CONSOLE_ID_cadence0	1
#define ZYNQMP_CONSOLE_ID_cadence1	2
#define ZYNQMP_CONSOLE_ID_dcc		3

#define ZYNQMP_CONSOLE_IS(con)	(ZYNQMP_CONSOLE_ID_ ## con == ZYNQMP_CONSOLE)

/* Firmware Image Package */
#define ZYNQMP_PRIMARY_CPU		0

/* Memory location options for Shared data and TSP in ZYNQMP */
#define ZYNQMP_IN_TRUSTED_SRAM		0
#define ZYNQMP_IN_TRUSTED_DRAM		1

/*******************************************************************************
 * ZYNQMP memory map related constants
 ******************************************************************************/
/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE		0xFF000000
#define DEVICE0_SIZE		0x00E00000
#define DEVICE1_BASE		0xF9000000
#define DEVICE1_SIZE		0x00800000

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
#define CRL_APB_BOOT_MODE_USER		(CRL_APB_BASE + 0x200)
#define CRL_APB_RESET_CTRL		(CRL_APB_BASE + 0x218)

#define CRL_APB_RPLL_CTRL_BYPASS	(1 << 3)

#define CRL_APB_RESET_CTRL_SOFT_RESET	(1 << 4)

#define CRL_APB_BOOT_MODE_MASK		(0xf << 0)
#define ZYNQMP_BOOTMODE_JTAG		0

/* system counter registers and bitfields */
#define IOU_SCNTRS_BASE			0xFF260000
#define IOU_SCNTRS_BASEFREQ		(IOU_SCNTRS_BASE + 0x20)

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
#define PMU_GLOBAL_GEN_STORAGE6		(PMU_GLOBAL_BASE + 0x48)
#define PMU_GLOBAL_REQ_PWRUP_STATUS	(PMU_GLOBAL_BASE + 0x110)
#define PMU_GLOBAL_REQ_PWRUP_EN		(PMU_GLOBAL_BASE + 0x118)
#define PMU_GLOBAL_REQ_PWRUP_DIS	(PMU_GLOBAL_BASE + 0x11c)
#define PMU_GLOBAL_REQ_PWRUP_TRIG	(PMU_GLOBAL_BASE + 0x120)

#define PMU_GLOBAL_CNTRL_FW_IS_PRESENT	(1 << 4)

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
#define ZYNQMP_UART1_BASE		0xFF010000

#if ZYNQMP_CONSOLE_IS(cadence)
# define ZYNQMP_UART_BASE	ZYNQMP_UART0_BASE
#elif ZYNQMP_CONSOLE_IS(cadence1)
# define ZYNQMP_UART_BASE	ZYNQMP_UART1_BASE
#else
# error "invalid ZYNQMP_CONSOLE"
#endif

#define PLAT_ARM_CRASH_UART_BASE	ZYNQMP_UART_BASE
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

/* Access control register defines */
#define ACTLR_EL3_L2ACTLR_BIT	(1 << 6)
#define ACTLR_EL3_CPUACTLR_BIT	(1 << 0)

#define IOU_SLCR_BASEADDR		0xFF180000

#endif /* __ZYNQMP_DEF_H__ */
