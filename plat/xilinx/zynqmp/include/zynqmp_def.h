/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ZYNQMP_DEF_H
#define ZYNQMP_DEF_H

#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

#define ZYNQMP_CONSOLE_ID_cadence	1
#define ZYNQMP_CONSOLE_ID_cadence0	1
#define ZYNQMP_CONSOLE_ID_cadence1	2
#define ZYNQMP_CONSOLE_ID_dcc		3

#define ZYNQMP_CONSOLE_IS(con)	(ZYNQMP_CONSOLE_ID_ ## con == ZYNQMP_CONSOLE)

/* Default counter frequency */
#define ZYNQMP_DEFAULT_COUNTER_FREQ	0U

/* Firmware Image Package */
#define ZYNQMP_PRIMARY_CPU		0

/* Memory location options for Shared data and TSP in ZYNQMP */
#define ZYNQMP_IN_TRUSTED_SRAM		0
#define ZYNQMP_IN_TRUSTED_DRAM		1

/*******************************************************************************
 * ZYNQMP memory map related constants
 ******************************************************************************/
/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE		U(0xFF000000)
#define DEVICE0_SIZE		U(0x00E00000)
#define DEVICE1_BASE		U(0xF9000000)
#define DEVICE1_SIZE		U(0x00800000)

/* For cpu reset APU space here too 0xFE5F1000 CRF_APB*/
#define CRF_APB_BASE		U(0xFD1A0000)
#define CRF_APB_SIZE		U(0x00600000)
#define CRF_APB_CLK_BASE	U(0xFD1A0020)

/* CRF registers and bitfields */
#define CRF_APB_RST_FPD_APU	(CRF_APB_BASE + 0X00000104)

#define CRF_APB_RST_FPD_APU_ACPU_RESET		(U(1) << 0)
#define CRF_APB_RST_FPD_APU_ACPU_PWRON_RESET	(U(1) << 10)

/* CRL registers and bitfields */
#define CRL_APB_BASE			U(0xFF5E0000)
#define CRL_APB_BOOT_MODE_USER		(CRL_APB_BASE + 0x200)
#define CRL_APB_RESET_CTRL		(CRL_APB_BASE + 0x218)
#define CRL_APB_RST_LPD_TOP		(CRL_APB_BASE + 0x23C)
#define CRL_APB_BOOT_PIN_CTRL		(CRL_APB_BASE + U(0x250))
#define CRL_APB_CLK_BASE		U(0xFF5E0020)

#define CRL_APB_RPU_AMBA_RESET		(U(1) << 2)
#define CRL_APB_RPLL_CTRL_BYPASS	(U(1) << 3)

#define CRL_APB_RESET_CTRL_SOFT_RESET	(U(1) << 4)

#define CRL_APB_BOOT_MODE_MASK		(U(0xf) << 0)
#define CRL_APB_BOOT_PIN_MASK		(U(0xf0f) << 0)
#define CRL_APB_BOOT_DRIVE_PIN_1_SHIFT	U(9)
#define CRL_APB_BOOT_ENABLE_PIN_1_SHIFT	U(1)
#define CRL_APB_BOOT_ENABLE_PIN_1	(U(0x1) << \
					CRL_APB_BOOT_ENABLE_PIN_1_SHIFT)
#define CRL_APB_BOOT_DRIVE_PIN_1	(U(0x1) << \
					CRL_APB_BOOT_DRIVE_PIN_1_SHIFT)
#define ZYNQMP_BOOTMODE_JTAG		U(0)
#define ZYNQMP_ULPI_RESET_VAL_HIGH	(CRL_APB_BOOT_ENABLE_PIN_1 | \
					 CRL_APB_BOOT_DRIVE_PIN_1)
#define ZYNQMP_ULPI_RESET_VAL_LOW	CRL_APB_BOOT_ENABLE_PIN_1

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

#if ZYNQMP_WDT_RESTART
#define IRQ_SEC_IPI_APU		67
#define IRQ_TTC3_1		77
#define TTC3_BASE_ADDR		0xFF140000
#define TTC3_INTR_REGISTER_1	(TTC3_BASE_ADDR + 0x54)
#define TTC3_INTR_ENABLE_1	(TTC3_BASE_ADDR + 0x60)
#endif

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

#if ZYNQMP_CONSOLE_IS(cadence) || ZYNQMP_CONSOLE_IS(dcc)
# define ZYNQMP_UART_BASE	ZYNQMP_UART0_BASE
#elif ZYNQMP_CONSOLE_IS(cadence1)
# define ZYNQMP_UART_BASE	ZYNQMP_UART1_BASE
#else
# error "invalid ZYNQMP_CONSOLE"
#endif

#define ZYNQMP_CRASH_UART_BASE		ZYNQMP_UART_BASE
/* impossible to call C routine how it is done now - hardcode any value */
#define ZYNQMP_CRASH_UART_CLK_IN_HZ	100000000 /* FIXME */
/* Must be non zero */
#define ZYNQMP_UART_BAUDRATE		115200

/* Silicon version detection */
#define ZYNQMP_SILICON_VER_MASK		0xF000
#define ZYNQMP_SILICON_VER_SHIFT	12
#define ZYNQMP_CSU_VERSION_SILICON	0
#define ZYNQMP_CSU_VERSION_QEMU		3

#define ZYNQMP_RTL_VER_MASK		0xFF0
#define ZYNQMP_RTL_VER_SHIFT		4

#define ZYNQMP_PS_VER_MASK		0xF
#define ZYNQMP_PS_VER_SHIFT		0

#define ZYNQMP_CSU_BASEADDR		0xFFCA0000
#define ZYNQMP_CSU_IDCODE_OFFSET	0x40

#define ZYNQMP_CSU_IDCODE_XILINX_ID_SHIFT	0
#define ZYNQMP_CSU_IDCODE_XILINX_ID_MASK	(0xFFF << \
					ZYNQMP_CSU_IDCODE_XILINX_ID_SHIFT)
#define ZYNQMP_CSU_IDCODE_XILINX_ID		0x093

#define ZYNQMP_CSU_IDCODE_SVD_SHIFT		12
#define ZYNQMP_CSU_IDCODE_SVD_MASK		(0x7 << \
						 ZYNQMP_CSU_IDCODE_SVD_SHIFT)
#define ZYNQMP_CSU_IDCODE_DEVICE_CODE_SHIFT	15
#define ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK	(0xF << \
					ZYNQMP_CSU_IDCODE_DEVICE_CODE_SHIFT)
#define ZYNQMP_CSU_IDCODE_SUB_FAMILY_SHIFT	19
#define ZYNQMP_CSU_IDCODE_SUB_FAMILY_MASK	(0x3 << \
					ZYNQMP_CSU_IDCODE_SUB_FAMILY_SHIFT)
#define ZYNQMP_CSU_IDCODE_FAMILY_SHIFT		21
#define ZYNQMP_CSU_IDCODE_FAMILY_MASK		(0x7F << \
					ZYNQMP_CSU_IDCODE_FAMILY_SHIFT)
#define ZYNQMP_CSU_IDCODE_FAMILY		0x23

#define ZYNQMP_CSU_IDCODE_REVISION_SHIFT	28
#define ZYNQMP_CSU_IDCODE_REVISION_MASK		(0xF << \
					ZYNQMP_CSU_IDCODE_REVISION_SHIFT)
#define ZYNQMP_CSU_IDCODE_REVISION		0

#define ZYNQMP_CSU_VERSION_OFFSET	0x44

/* Efuse */
#define EFUSE_BASEADDR		0xFFCC0000
#define EFUSE_IPDISABLE_OFFSET	0x1018
#define EFUSE_IPDISABLE_VERSION	0x1FFU
#define ZYNQMP_EFUSE_IPDISABLE_SHIFT	20

/* Access control register defines */
#define ACTLR_EL3_L2ACTLR_BIT	(1 << 6)
#define ACTLR_EL3_CPUACTLR_BIT	(1 << 0)

#define FPD_SLCR_BASEADDR		U(0xFD610000)
#define IOU_SLCR_BASEADDR		U(0xFF180000)

#define ZYNQMP_RPU_GLBL_CNTL			U(0xFF9A0000)
#define ZYNQMP_RPU0_CFG				U(0xFF9A0100)
#define ZYNQMP_RPU1_CFG				U(0xFF9A0200)
#define ZYNQMP_SLSPLIT_MASK			U(0x08)
#define ZYNQMP_TCM_COMB_MASK			U(0x40)
#define ZYNQMP_SLCLAMP_MASK			U(0x10)
#define ZYNQMP_VINITHI_MASK			U(0x04)

/* Tap delay bypass */
#define IOU_TAPDLY_BYPASS			U(0XFF180390)
#define TAP_DELAY_MASK				U(0x7)

/* SGMII mode */
#define IOU_GEM_CTRL				U(0xFF180360)
#define IOU_GEM_CLK_CTRL			U(0xFF180308)
#define SGMII_SD_MASK				U(0x3)
#define SGMII_SD_OFFSET				U(2)
#define SGMII_PCS_SD_0				U(0x0)
#define SGMII_PCS_SD_1				U(0x1)
#define SGMII_PCS_SD_PHY			U(0x2)
#define GEM_SGMII_MASK				U(0x4)
#define GEM_CLK_CTRL_MASK			U(0xF)
#define GEM_CLK_CTRL_OFFSET			U(5)
#define GEM_RX_SRC_SEL_GTR			U(0x1)
#define GEM_SGMII_MODE				U(0x4)

/* SD DLL reset */
#define ZYNQMP_SD_DLL_CTRL			U(0xFF180358)
#define ZYNQMP_SD0_DLL_RST_MASK			U(0x00000004)
#define ZYNQMP_SD0_DLL_RST			U(0x00000004)
#define ZYNQMP_SD1_DLL_RST_MASK			U(0x00040000)
#define ZYNQMP_SD1_DLL_RST			U(0x00040000)

/* SD tap delay */
#define ZYNQMP_SD_DLL_CTRL			U(0xFF180358)
#define ZYNQMP_SD_ITAP_DLY			U(0xFF180314)
#define ZYNQMP_SD_OTAP_DLY			U(0xFF180318)
#define ZYNQMP_SD_TAP_OFFSET			U(16)
#define ZYNQMP_SD_ITAPCHGWIN_MASK		U(0x200)
#define ZYNQMP_SD_ITAPCHGWIN			U(0x200)
#define ZYNQMP_SD_ITAPDLYENA_MASK		U(0x100)
#define ZYNQMP_SD_ITAPDLYENA			U(0x100)
#define ZYNQMP_SD_ITAPDLYSEL_MASK		U(0xFF)
#define ZYNQMP_SD_OTAPDLYSEL_MASK		U(0x3F)
#define ZYNQMP_SD_OTAPDLYENA_MASK		U(0x40)
#define ZYNQMP_SD_OTAPDLYENA			U(0x40)

/* Clock control registers */
/* Full power domain clocks */
#define CRF_APB_APLL_CTRL		(CRF_APB_CLK_BASE + 0x00)
#define CRF_APB_DPLL_CTRL		(CRF_APB_CLK_BASE + 0x0c)
#define CRF_APB_VPLL_CTRL		(CRF_APB_CLK_BASE + 0x18)
#define CRF_APB_PLL_STATUS		(CRF_APB_CLK_BASE + 0x24)
#define CRF_APB_APLL_TO_LPD_CTRL	(CRF_APB_CLK_BASE + 0x28)
#define CRF_APB_DPLL_TO_LPD_CTRL	(CRF_APB_CLK_BASE + 0x2c)
#define CRF_APB_VPLL_TO_LPD_CTRL	(CRF_APB_CLK_BASE + 0x30)
/* Peripheral clocks */
#define CRF_APB_ACPU_CTRL		(CRF_APB_CLK_BASE + 0x40)
#define CRF_APB_DBG_TRACE_CTRL		(CRF_APB_CLK_BASE + 0x44)
#define CRF_APB_DBG_FPD_CTRL		(CRF_APB_CLK_BASE + 0x48)
#define CRF_APB_DP_VIDEO_REF_CTRL	(CRF_APB_CLK_BASE + 0x50)
#define CRF_APB_DP_AUDIO_REF_CTRL	(CRF_APB_CLK_BASE + 0x54)
#define CRF_APB_DP_STC_REF_CTRL		(CRF_APB_CLK_BASE + 0x5c)
#define CRF_APB_DDR_CTRL		(CRF_APB_CLK_BASE + 0x60)
#define CRF_APB_GPU_REF_CTRL		(CRF_APB_CLK_BASE + 0x64)
#define CRF_APB_SATA_REF_CTRL		(CRF_APB_CLK_BASE + 0x80)
#define CRF_APB_PCIE_REF_CTRL		(CRF_APB_CLK_BASE + 0x94)
#define CRF_APB_GDMA_REF_CTRL		(CRF_APB_CLK_BASE + 0x98)
#define CRF_APB_DPDMA_REF_CTRL		(CRF_APB_CLK_BASE + 0x9c)
#define CRF_APB_TOPSW_MAIN_CTRL		(CRF_APB_CLK_BASE + 0xa0)
#define CRF_APB_TOPSW_LSBUS_CTRL	(CRF_APB_CLK_BASE + 0xa4)
#define CRF_APB_GTGREF0_REF_CTRL	(CRF_APB_CLK_BASE + 0xa8)
#define CRF_APB_DBG_TSTMP_CTRL		(CRF_APB_CLK_BASE + 0xd8)

/* Low power domain clocks */
#define CRL_APB_IOPLL_CTRL		(CRL_APB_CLK_BASE + 0x00)
#define CRL_APB_RPLL_CTRL		(CRL_APB_CLK_BASE + 0x10)
#define CRL_APB_PLL_STATUS		(CRL_APB_CLK_BASE + 0x20)
#define CRL_APB_IOPLL_TO_FPD_CTRL	(CRL_APB_CLK_BASE + 0x24)
#define CRL_APB_RPLL_TO_FPD_CTRL	(CRL_APB_CLK_BASE + 0x28)
/* Peripheral clocks */
#define CRL_APB_USB3_DUAL_REF_CTRL	(CRL_APB_CLK_BASE + 0x2c)
#define CRL_APB_GEM0_REF_CTRL		(CRL_APB_CLK_BASE + 0x30)
#define CRL_APB_GEM1_REF_CTRL		(CRL_APB_CLK_BASE + 0x34)
#define CRL_APB_GEM2_REF_CTRL		(CRL_APB_CLK_BASE + 0x38)
#define CRL_APB_GEM3_REF_CTRL		(CRL_APB_CLK_BASE + 0x3c)
#define CRL_APB_USB0_BUS_REF_CTRL	(CRL_APB_CLK_BASE + 0x40)
#define CRL_APB_USB1_BUS_REF_CTRL	(CRL_APB_CLK_BASE + 0x44)
#define CRL_APB_QSPI_REF_CTRL		(CRL_APB_CLK_BASE + 0x48)
#define CRL_APB_SDIO0_REF_CTRL		(CRL_APB_CLK_BASE + 0x4c)
#define CRL_APB_SDIO1_REF_CTRL		(CRL_APB_CLK_BASE + 0x50)
#define CRL_APB_UART0_REF_CTRL		(CRL_APB_CLK_BASE + 0x54)
#define CRL_APB_UART1_REF_CTRL		(CRL_APB_CLK_BASE + 0x58)
#define CRL_APB_SPI0_REF_CTRL		(CRL_APB_CLK_BASE + 0x5c)
#define CRL_APB_SPI1_REF_CTRL		(CRL_APB_CLK_BASE + 0x60)
#define CRL_APB_CAN0_REF_CTRL		(CRL_APB_CLK_BASE + 0x64)
#define CRL_APB_CAN1_REF_CTRL		(CRL_APB_CLK_BASE + 0x68)
#define CRL_APB_CPU_R5_CTRL		(CRL_APB_CLK_BASE + 0x70)
#define CRL_APB_IOU_SWITCH_CTRL		(CRL_APB_CLK_BASE + 0x7c)
#define CRL_APB_CSU_PLL_CTRL		(CRL_APB_CLK_BASE + 0x80)
#define CRL_APB_PCAP_CTRL		(CRL_APB_CLK_BASE + 0x84)
#define CRL_APB_LPD_SWITCH_CTRL		(CRL_APB_CLK_BASE + 0x88)
#define CRL_APB_LPD_LSBUS_CTRL		(CRL_APB_CLK_BASE + 0x8c)
#define CRL_APB_DBG_LPD_CTRL		(CRL_APB_CLK_BASE + 0x90)
#define CRL_APB_NAND_REF_CTRL		(CRL_APB_CLK_BASE + 0x94)
#define CRL_APB_ADMA_REF_CTRL		(CRL_APB_CLK_BASE + 0x98)
#define CRL_APB_PL0_REF_CTRL		(CRL_APB_CLK_BASE + 0xa0)
#define CRL_APB_PL1_REF_CTRL		(CRL_APB_CLK_BASE + 0xa4)
#define CRL_APB_PL2_REF_CTRL		(CRL_APB_CLK_BASE + 0xa8)
#define CRL_APB_PL3_REF_CTRL		(CRL_APB_CLK_BASE + 0xac)
#define CRL_APB_PL0_THR_CNT		(CRL_APB_CLK_BASE + 0xb4)
#define CRL_APB_PL1_THR_CNT		(CRL_APB_CLK_BASE + 0xbc)
#define CRL_APB_PL2_THR_CNT		(CRL_APB_CLK_BASE + 0xc4)
#define CRL_APB_PL3_THR_CNT		(CRL_APB_CLK_BASE + 0xdc)
#define CRL_APB_GEM_TSU_REF_CTRL	(CRL_APB_CLK_BASE + 0xe0)
#define CRL_APB_DLL_REF_CTRL		(CRL_APB_CLK_BASE + 0xe4)
#define CRL_APB_AMS_REF_CTRL		(CRL_APB_CLK_BASE + 0xe8)
#define CRL_APB_I2C0_REF_CTRL		(CRL_APB_CLK_BASE + 0x100)
#define CRL_APB_I2C1_REF_CTRL		(CRL_APB_CLK_BASE + 0x104)
#define CRL_APB_TIMESTAMP_REF_CTRL	(CRL_APB_CLK_BASE + 0x108)
#define IOU_SLCR_GEM_CLK_CTRL		(IOU_SLCR_BASEADDR + 0x308)
#define IOU_SLCR_CAN_MIO_CTRL		(IOU_SLCR_BASEADDR + 0x304)
#define FPD_SLCR_WDT_CLK_SEL		(FPD_SLCR_BASEADDR + 0x100)
#define IOU_SLCR_WDT_CLK_SEL		(IOU_SLCR_BASEADDR + 0x300)

/* Global general storage register base address */
#define GGS_BASEADDR		(0xFFD80030U)
#define GGS_NUM_REGS		U(4)

/* Persistent global general storage register base address */
#define PGGS_BASEADDR		(0xFFD80050U)
#define PGGS_NUM_REGS		U(4)

/* PMU GGS4 register 4 is used for warm restart boot health status */
#define PMU_GLOBAL_GEN_STORAGE4			(GGS_BASEADDR + 0x10)
/* Warm restart boot health status mask */
#define PM_BOOT_HEALTH_STATUS_MASK		U(0x01)
/* WDT restart scope shift and mask */
#define RESTART_SCOPE_SHIFT			(3)
#define RESTART_SCOPE_MASK			(0x3U << RESTART_SCOPE_SHIFT)

/*AFI registers */
#define  AFIFM6_WRCTRL		U(13)
#define  FABRIC_WIDTH		U(3)

/* CSUDMA Module Base Address*/
#define CSUDMA_BASE		0xFFC80000

/* RSA-CORE Module Base Address*/
#define RSA_CORE_BASE		0xFFCE0000

#endif /* ZYNQMP_DEF_H */
